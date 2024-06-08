#include "ModelLoader.h"
#include <unordered_map>


#undef min
#undef max
#include <glm/detail/type_quat.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

#include <fastgltf/glm_element_traits.hpp>
#include <fastgltf/tools.hpp>
#include <fastgltf/util.hpp>
#include <fastgltf/core.hpp>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include "Core/Logger.h"
#include "MaterialManager.h"
#include "Mesh.h"
#include "Vertex.h"
#include "Scene/Scene.h"



namespace GLTFLoader {
    void LoadGLTF(std::string_view filePath, Scene *scene, VulkanContext *vulkanContext)
    {
        LogInfo("Loading GLTF: " + std::string(filePath));
        auto gltf_opt = std::move(Load( VulkanContext::GetAssetPath(std::string(filePath)).generic_string() ));
        if(!gltf_opt.has_value())
        {
            LogError("Failed to load GLTF: " + std::string(filePath));
            return;
        }

        fastgltf::Asset &gltf = gltf_opt.value();

        // Create the materials
        const size_t startMaterial = MaterialManager::GetMaterials().size();
        CreateMaterials(gltf, vulkanContext);


        std::vector<Mesh*> createdMeshes;
        std::vector<uint32_t> indices{};
        std::vector<Vertex> vertices{};

        // For Every mesh
        for (fastgltf::Mesh &mesh: gltf.meshes)
        {
            indices.clear();
            vertices.clear();

            std::size_t submeshMaterialIndex = 0;
            //For every submesh
            for (auto &&subMesh: mesh.primitives)
            {
                const uint32_t indexSize = static_cast<uint32_t>(indices.size());
                const int32_t vertexSize = static_cast<int32_t>(vertices.size());

                submeshMaterialIndex = subMesh.materialIndex.value();

                // load indexes
                {
                    fastgltf::Accessor &indexaccessor = gltf.accessors[subMesh.indicesAccessor.value()];

                    // reserve the space for the new indices
                    indices.reserve(indexSize + indexaccessor.count);

                    //Add the indices to the indices vector
                    fastgltf::iterateAccessor<std::uint32_t>(gltf, indexaccessor, [&](std::uint32_t idx)
                    {
                        indices.push_back(idx );
                    });
                }


                // load vertex positions
                {
                    fastgltf::Accessor &posAccessor = gltf.accessors[subMesh.findAttribute("POSITION")->second];



                    //Resize the vertices vector to fit the new vertices
                    vertices.resize(vertexSize + posAccessor.count);

                    ///Push the positions
                    fastgltf::iterateAccessorWithIndex<glm::vec3>(gltf, posAccessor,
                                                                  [&](const glm::vec3 pos, const size_t index)
                                                                  {
                                                                      vertices[vertexSize + index].pos = pos;
                                                                  });
                }


                // load vertex normals
                const auto normals = subMesh.findAttribute("NORMAL");
                if (normals != subMesh.attributes.end()) {
                    fastgltf::iterateAccessorWithIndex<glm::vec3>(gltf, gltf.accessors[(*normals).second],
                                                                  [&](const glm::vec3 v, const size_t index)
                                                                  {
                                                                      vertices[vertexSize + index].normal = v;
                                                                  });
                }

                // load UVs
                const auto uv = subMesh.findAttribute("TEXCOORD_0");
                if (uv != subMesh.attributes.end())
                {
                    fastgltf::iterateAccessorWithIndex<glm::vec2>(gltf, gltf.accessors[(*uv).second],
                            [&](glm::vec2 v, size_t index) {
                                vertices[vertexSize + index].texCoord = v;
                            });
                }

                // // Calculate Tangents
                for (size_t i{}; i < indices.size(); i += 3) {
                    const Vertex &v0 = vertices[indices[i + 0]];
                    const Vertex &v1 = vertices[indices[i + 1]];
                    const Vertex &v2 = vertices[indices[i + 2]];

                    const glm::vec3 edge1 = v1.pos - v0.pos;
                    const glm::vec3 edge2 = v2.pos - v0.pos;

                    const glm::vec2 deltaUV1 = v1.texCoord - v0.texCoord;
                    const glm::vec2 deltaUV2 = v2.texCoord - v0.texCoord;

                    const float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

                    glm::vec3 tangent;
                    tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
                    tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
                    tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

                    // Normalize the Tangent
                    tangent = glm::normalize(tangent);

                    // Add the Tangent to the vertices
                    vertices[indices[i + 0]].tangent += tangent;
                    vertices[indices[i + 1]].tangent += tangent;
                    vertices[indices[i + 2]].tangent += tangent;
                }
            }


            //TODO:This should be moved to the inner loop, The mesh class should be split up in a mesh that holds a primitive.
            //Each primitive will holds its own draw call?
            //From a shared index and vertex buffer?

            // Get the correct material
            size_t materialIndex = startMaterial + submeshMaterialIndex;
            // Create the Mesh
            std::unique_ptr<Mesh> newmesh = std::make_unique<Mesh>(vertices, indices, MaterialManager::GetMaterials()[materialIndex], mesh.name.c_str(), 0, 0);
            createdMeshes.push_back(newmesh.get());
            scene->AddMesh(std::move(newmesh));
        }

        //Setup TRS
        for(fastgltf::Node& node : gltf.nodes)
        {
            if(node.meshIndex.has_value())
            {
                Mesh *newNode = createdMeshes[node.meshIndex.value()];
                std::variant<fastgltf::TRS, std::array<float, 16>> transform = node.transform;
                if (std::holds_alternative<fastgltf::TRS>(transform))
                {
                    fastgltf::TRS trs = std::get<fastgltf::TRS>(transform);

                    // Create glm vectors from the TRS structure
                    glm::vec3 translation(trs.translation[0], trs.translation[1], trs.translation[2]);
                    glm::quat rotation(trs.rotation[3], trs.rotation[0], trs.rotation[1], trs.rotation[2]);
                    glm::vec3 scale(trs.scale[0], trs.scale[1], trs.scale[2]);

                    // Create the transformation matrix
                    glm::mat4 transformMatrix = glm::translate(glm::mat4(1.0f), translation) * glm::mat4_cast(rotation) * glm::scale(glm::mat4(1.0f), scale);


                    // Create a rotation matrix to rotate 90 degrees around the x-axis
                    glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

                    // Multiply the rotation matrix with the transformation matrix
                    transformMatrix = rotationMatrix * transformMatrix;

                    newNode->SetTransform(transformMatrix);

                } else if (std::holds_alternative<std::array<float, 16>>(transform))
                {
                    std::array<float, 16> matrix = std::get<std::array<float, 16>>(transform);

                    glm::mat4 transformMatrix = glm::make_mat4(matrix.data());
                    newNode->SetTransform(transformMatrix);
                }
            }
        }
    }



    std::optional<fastgltf::Asset> Load(std::string_view filePath)
    {
        std::optional<fastgltf::Asset> gltf = std::nullopt;
        if (!std::filesystem::exists(filePath))
        {
            LogError("File does not exist: " + std::string(filePath));
            return gltf;
        }


        auto gltfOptions = fastgltf::Options::DontRequireValidAssetMember | fastgltf::Options::AllowDouble | fastgltf::Options::LoadGLBBuffers | fastgltf::Options::LoadExternalBuffers | fastgltf::Options::GenerateMeshIndices;
        //gltfOptions |= fastgltf::Options::LoadExternalImages;

        fastgltf::Parser parser{};
        fastgltf::GltfDataBuffer data;
        data.loadFromFile(filePath);


        std::filesystem::path path = filePath;

        // Check if the file is a glTF or a GLB
        if (fastgltf::GltfType type = determineGltfFileType(&data); type == fastgltf::GltfType::glTF)
        {
            fastgltf::Expected<fastgltf::Asset> isSuccesfullyLoaded = parser.loadGltf(&data, path.parent_path(), gltfOptions);
            if (isSuccesfullyLoaded) gltf = std::move(isSuccesfullyLoaded.get());
            else LogError("Failed to load GLTF: " + std::to_string(fastgltf::to_underlying(isSuccesfullyLoaded.error())));
            return gltf;


        //Load GLB
        } else if (type == fastgltf::GltfType::GLB) {

            fastgltf::Expected<fastgltf::Asset> isSuccesfullyLoaded = parser.loadGltfBinary(&data, path.parent_path(), gltfOptions);
            if (isSuccesfullyLoaded)  gltf = std::move(isSuccesfullyLoaded.get());
            else LogError("Failed to load GLTF: " + std::to_string(fastgltf::to_underlying(isSuccesfullyLoaded.error())));
            return gltf;
        }
        LogError("Failed to determine glTF container");
        return gltf;
    }


    void CreateMaterials(const fastgltf::Asset &gltf, VulkanContext* vulkanContext)
    {
        // Load Textures
        std::vector<std::variant<std::filesystem::path, ImageInMemory>> images;

        for (const fastgltf::Image &texture: gltf.images)
        {
            fastgltf::DataSource dataa = texture.data;
            if (const auto uri = std::get_if<fastgltf::sources::URI>(&dataa))
            {
                images.emplace_back(VulkanContext::GetAssetPath(std::string(uri->uri.string())).generic_string());
            }
            else if(auto array = std::get_if<fastgltf::sources::Array>(&dataa))
            {

                ImageInMemory loadedImage{};
                if(array->mimeType == fastgltf::MimeType::KTX2)
                {
                    auto pD = array->bytes;
                    ktxTexture* texturePtr{};
                    auto sources = ktx::CreateImageFromMemory(array->bytes.data(), array->bytes.size(),loadedImage.imageSize, loadedImage.mipLevels, &texturePtr);

                    loadedImage.texture = texturePtr;
                    loadedImage.staginBuffer = sources.first;
                    loadedImage.stagingBufferMemory = sources.second;

                }else if(array->mimeType == fastgltf::MimeType::PNG || array->mimeType == fastgltf::MimeType::JPEG ||array->mimeType == fastgltf::MimeType::None)
                {
                    auto sources = stbi::CreateImageFromMemory(array->bytes.data(), array->bytes.size(), loadedImage.imageSize, loadedImage.mipLevels);
                    loadedImage.staginBuffer = sources.first;
                     loadedImage.stagingBufferMemory = sources.second;
                }else {
                    LogError("Unsuported embeded texture format in gtlf");
                }

                images.emplace_back(loadedImage);

            }

            else if(auto customBuffer = std::get_if<fastgltf::sources::CustomBuffer>(&dataa))
            {
                LogError("CustomBuffer not supported yet");
            }
            else if(auto byteView = std::get_if<fastgltf::sources::ByteView>(&dataa))
            {
                LogError("ByteView not supported yet");
            }
            else if(auto fallback = std::get_if<fastgltf::sources::Fallback>(&dataa))
            {
                LogError("Fallback not supported yet");
            }
            else if (auto bufferView = std::get_if<fastgltf::sources::BufferView>(&dataa))
            {
                LogError("BufferView not supported yet");
            }
        }


        for (const fastgltf::Material &mat: gltf.materials)
        {
            auto newMaterial = MaterialManager::CreateMaterial(vulkanContext, "shader.vert", "PBR_Graypacked.frag", mat.name.c_str());
            auto *ubo = newMaterial->GetDescriptorSet()->AddUniformBuffer(0);
            ubo->AddVariable(glm::vec4{1});
            ubo->AddVariable(glm::vec4{1});

            // Load Albedo
            if (mat.pbrData.baseColorTexture.has_value())
            {
                size_t img = gltf.textures[mat.pbrData.baseColorTexture.value().textureIndex].imageIndex.value();
                newMaterial->GetDescriptorSet()->AddTexture(1, images[img], vulkanContext, ColorType::SRGB);
            }else{
                newMaterial->GetDescriptorSet()->AddTexture(1, "Assets/white.ktx", vulkanContext, ColorType::LINEAR);
            }

            // Load Normal
            if (mat.normalTexture.has_value()) {
                size_t img = gltf.textures[mat.normalTexture.value().textureIndex].imageIndex.value();
                newMaterial->GetDescriptorSet()->AddTexture(2, images[img], vulkanContext, ColorType::LINEAR);
            }else
            {
                newMaterial->GetDescriptorSet()->AddTexture(2, "Assets/white.ktx", vulkanContext, ColorType::LINEAR);
            }

            // Load graypacked metal/roughness
            if (mat.pbrData.metallicRoughnessTexture.has_value())
            {
                size_t img = gltf.textures[mat.pbrData.metallicRoughnessTexture.value().textureIndex].imageIndex.value();

                newMaterial->GetDescriptorSet()->AddTexture(3, images[img], vulkanContext, ColorType::LINEAR);
            }else
            {
                newMaterial->GetDescriptorSet()->AddTexture(3, "Assets/white.ktx", vulkanContext, ColorType::LINEAR);
            }

            // TODO: Scene->GetCubeMap();
            newMaterial->GetDescriptorSet()->AddTexture(4, "cubemap_vulkan.ktx", vulkanContext, ColorType::SRGB, TextureType::TEXTURE_CUBE);
        }
    }


    VkFilter GetVkFilter(fastgltf::Filter filter)
    {
        switch (filter) {
            // nearest samplers
            case fastgltf::Filter::Nearest:
            case fastgltf::Filter::NearestMipMapNearest:
            case fastgltf::Filter::NearestMipMapLinear:
                return VK_FILTER_NEAREST;

            // linear samplers
            case fastgltf::Filter::Linear:
            case fastgltf::Filter::LinearMipMapNearest:
            case fastgltf::Filter::LinearMipMapLinear:
            default:
                return VK_FILTER_LINEAR;
        }
    }
    VkSamplerMipmapMode GetVkSamplerMipmapMode(fastgltf::Filter mode) {
        switch (mode) {
            case fastgltf::Filter::NearestMipMapNearest:
            case fastgltf::Filter::LinearMipMapNearest:
                return VK_SAMPLER_MIPMAP_MODE_NEAREST;

            case fastgltf::Filter::NearestMipMapLinear:
            case fastgltf::Filter::LinearMipMapLinear:
            default:
                return VK_SAMPLER_MIPMAP_MODE_LINEAR;
        }
    }


} // namespace GLTFLoader


// TODO Move to a seperate thread
namespace ObjLoader
{
	void LoadObj(const std::string& filePath, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices)
	{
        const auto path = VulkanContext::GetAssetPath(filePath);


		tinyobj::attrib_t attributes;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string errors;
		std::string warnings;

		const bool success = tinyobj::LoadObj(&attributes, &shapes, &materials, &warnings, &errors, path.generic_string().c_str());


		if (!success)
		{
			LogError("Failed to load: " + path.generic_string() + " with the following errors: " + errors);
		    return;
		}

	    LogWarning(warnings);

	    std::unordered_map<Vertex, uint32_t, VertexHasher> uniqueVertices{};
		for (auto& shape : shapes)
		{
			tinyobj::mesh_t& mesh = shape.mesh;

			for (auto i : mesh.indices)
			{
				glm::vec3 position =
				{
					attributes.vertices[3 * i.vertex_index],
					attributes.vertices[3 * i.vertex_index + 1],
					attributes.vertices[3 * i.vertex_index + 2]
				};

				glm::vec3 normal{};
				if(i.normal_index != -1)
				{
				    if(3 * i.normal_index + 2 >= attributes.vertices.size())
				    {
				        LogError("This is not a supported .Obj file! The Normals Index is out of bounds!");
				        return;
				    }

					normal =
					{
						attributes.vertices[3 * i.normal_index],
						attributes.vertices[3 * i.normal_index + 1],
						attributes.vertices[3 * i.normal_index + 2]
					};
				}else {
				    LogInfo("Normal Index Out of bounds!");
				}

				glm::vec2 texCoord{};
				if(i.texcoord_index != -1)
				{
					texCoord =
					{
						attributes.texcoords[2 * i.texcoord_index + 0],
						1.0f - attributes.texcoords[2 * i.texcoord_index + 1]
					};
				}else {
				    LogInfo("Texcoord Index Out of bounds!");
				}
	


				const Vertex vertex = { position, normal, {}, texCoord };

				if (!uniqueVertices.contains(vertex))
				{
					uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
					vertices.push_back(vertex);
				}

				indices.push_back(uniqueVertices[vertex]);
			}
		}


        //Caclulate Tangents
	    for (size_t i{}; i < indices.size(); i += 3)
	    {
	        const Vertex &v0 = vertices[indices[i + 0]];
	        const Vertex &v1 = vertices[indices[i + 1]];
	        const Vertex &v2 = vertices[indices[i + 2]];

	        const glm::vec3 edge1 = v1.pos - v0.pos;
	        const glm::vec3 edge2 = v2.pos - v0.pos;

	        const glm::vec2 deltaUV1 = v1.texCoord - v0.texCoord;
	        const glm::vec2 deltaUV2 = v2.texCoord - v0.texCoord;

	        const float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

	        glm::vec3 tangent;
	        tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
	        tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
	        tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

	        // Normalize the Tangent
	        tangent = glm::normalize(tangent);

	        // Add the Tangent to the vertices
	        vertices[indices[i + 0]].tangent += tangent;
	        vertices[indices[i + 1]].tangent += tangent;
	        vertices[indices[i + 2]].tangent += tangent;
	    }


	    LogInfo("Loaded: " + path.generic_string());
	}
}
