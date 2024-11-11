#include "ModelLoader.h"
#include <unordered_map>



#undef min
#undef max
#undef None
#define GLM_ENABLE_EXPERIMENTAL
#include <fastgltf/core.hpp>
#include <fastgltf/glm_element_traits.hpp>
#include <fastgltf/tools.hpp>
#include <fastgltf/util.hpp>
#include <glm/detail/type_quat.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include "MaterialManager.h"
#include "Mesh.h"
#include "Vertex.h"
#include "Core/Logger.h"
#include "Scene/Scene.h"
#include "Scene/SceneManager.h"


namespace GLTFLoader
{
	void LoadGLTF(std::string_view filePath, Scene *scene, VulkanContext *vulkanContext)
	{
		LogInfo("Loading GLTF: " + std::string(filePath));

		const std::filesystem::path fullfilePath = VulkanContext::GetAssetPath() / filePath;
		std::optional<fastgltf::Asset> gltfOpt = Load(fullfilePath.generic_string());

		if (!gltfOpt)
		{
			LogError("Failed to load GLTF: " + std::string(filePath));
			return;
		}

		fastgltf::Asset &gltf = gltfOpt.value();

		std::vector<std::string> createdMaterialNames;
		CreateMaterials(gltf, vulkanContext, createdMaterialNames);

		// Structure for holding shared buffers and mesh processing context
		struct MeshData
		{
			std::vector<Mesh *> meshes;
			std::vector<uint32_t> indices;
			std::vector<Vertex> vertices;
			std::vector<Primitive> primitives;
		} meshData;

		auto processSubMesh = [&](fastgltf::Primitive &subMesh, size_t vertexOffset, size_t indexOffset)
		{
			Primitive primitive{};
			primitive.material = MaterialManager::GetMaterial(createdMaterialNames[subMesh.materialIndex.value()]);
			primitive.firstIndex = static_cast<uint32_t>(indexOffset);

			// Load indices
			fastgltf::Accessor &indexAccessor = gltf.accessors[subMesh.indicesAccessor.value()];
			meshData.indices.reserve(indexOffset + indexAccessor.count);
			fastgltf::iterateAccessor<std::uint32_t>(gltf, indexAccessor, [&](std::uint32_t idx)
			{
				meshData.indices.push_back(idx + vertexOffset);
			});

			// Load vertex positions
			fastgltf::Accessor &posAccessor = gltf.accessors[subMesh.findAttribute("POSITION")->second];
			meshData.vertices.resize(vertexOffset + posAccessor.count);
			fastgltf::iterateAccessorWithIndex<glm::vec3>(gltf, posAccessor, [&](const glm::vec3 &pos, size_t index)
			{
				meshData.vertices[vertexOffset + index].pos = pos;
			});

			// Load vertex normals
			if (auto normals = subMesh.findAttribute("NORMAL"); normals != subMesh.attributes.end())
			{
				fastgltf::iterateAccessorWithIndex<glm::vec3>(gltf, gltf.accessors[normals->second], [&](const glm::vec3 &normal, size_t index)
				{
					meshData.vertices[vertexOffset + index].normal = normal;
				});
			}

			// Load UVs
			if (auto uv = subMesh.findAttribute("TEXCOORD_0"); uv != subMesh.attributes.end())
			{
				fastgltf::iterateAccessorWithIndex<glm::vec2>(gltf, gltf.accessors[uv->second], [&](const glm::vec2 &uvCoord, size_t index)
				{
					meshData.vertices[vertexOffset + index].texCoord = uvCoord;
				});
			}

			// Calculate tangents
			for (size_t i = indexOffset; i < meshData.indices.size(); i += 3)
			{
				const Vertex &v0 = meshData.vertices[meshData.indices[i]];
				const Vertex &v1 = meshData.vertices[meshData.indices[i + 1]];
				const Vertex &v2 = meshData.vertices[meshData.indices[i + 2]];

				glm::vec3 edge1 = v1.pos - v0.pos;
				glm::vec3 edge2 = v2.pos - v0.pos;
				glm::vec2 deltaUV1 = v1.texCoord - v0.texCoord;
				glm::vec2 deltaUV2 = v2.texCoord - v0.texCoord;

				float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
				glm::vec3 tangent{f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x), f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y), f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z)};

				tangent = glm::normalize(tangent);

				meshData.vertices[meshData.indices[i]].tangent += tangent;
				meshData.vertices[meshData.indices[i + 1]].tangent += tangent;
				meshData.vertices[meshData.indices[i + 2]].tangent += tangent;
			}

			primitive.indexCount = static_cast<uint32_t>(meshData.indices.size() - indexOffset);
			meshData.primitives.emplace_back(std::move(primitive));
		};

		// Iterate through all meshes
		for (fastgltf::Mesh &mesh : gltf.meshes)
		{
			meshData.indices.clear();
			meshData.vertices.clear();
			meshData.primitives.clear();

			size_t vertexOffset = 0, indexOffset = 0;
			for (auto &subMesh : mesh.primitives)
			{
				processSubMesh(subMesh, vertexOffset, indexOffset);
				vertexOffset = meshData.vertices.size();
				indexOffset = meshData.indices.size();
			}

			auto newMesh = std::make_unique<Mesh>(meshData.vertices, meshData.indices, mesh.name.c_str(), meshData.primitives, 0);
			meshData.meshes.push_back(newMesh.get());
			scene->AddMesh(std::move(newMesh));
		}

		// Setup Transform, Rotation, and Scale (TRS)
		for (fastgltf::Node &node : gltf.nodes)
		{
			if (node.meshIndex)
			{
				Mesh *meshNode = meshData.meshes[node.meshIndex.value()];
				auto transform = node.transform;

				glm::mat4 transformMatrix = std::holds_alternative<fastgltf::TRS>(transform) ? ComputeTransformMatrix(std::get<fastgltf::TRS>(transform)) : glm::make_mat4(std::get<std::array<float, 16>>(transform).data());

				// Rotate 90 degrees around the x-axis
				transformMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)) * transformMatrix;

				meshNode->SetTransform(transformMatrix);
			}
		}
	}

	// Helper function to compute a transform matrix from TRS
	glm::mat4 ComputeTransformMatrix(const fastgltf::TRS &trs)
	{
		glm::vec3 translation(trs.translation[0], trs.translation[1], trs.translation[2]);
		glm::quat rotation(trs.rotation[3], trs.rotation[0], trs.rotation[1], trs.rotation[2]);
		glm::vec3 scale(trs.scale[0], trs.scale[1], trs.scale[2]);

		return glm::translate(glm::mat4(1.0f), translation) * glm::mat4_cast(rotation) * glm::scale(glm::mat4(1.0f), scale);
	}


	std::optional<fastgltf::Asset> Load(std::string_view filePath)
	{
		if (!std::filesystem::exists(filePath))
		{
			LogError("File does not exist: " + std::string(filePath));
			return std::nullopt;
		}

		fastgltf::Parser parser;
		fastgltf::GltfDataBuffer data;
		data.loadFromFile(filePath);

		fastgltf::Options gltfOptions = fastgltf::Options::DontRequireValidAssetMember | fastgltf::Options::AllowDouble | fastgltf::Options::LoadGLBBuffers | fastgltf::Options::LoadExternalBuffers | fastgltf::Options::GenerateMeshIndices;

		struct LoadContext
		{
			fastgltf::Parser &parser;
			fastgltf::GltfDataBuffer &data;
			const std::filesystem::path &parentPath;
			fastgltf::Options options;
		} context{parser, data, std::filesystem::path(filePath).parent_path(), gltfOptions};

		auto loadAsset = [](const LoadContext &ctx, fastgltf::GltfType type) -> fastgltf::Expected<fastgltf::Asset>
		{
			if (type == fastgltf::GltfType::glTF)
			{
				return ctx.parser.loadGltf(&ctx.data, ctx.parentPath, ctx.options);
			}
			if (type == fastgltf::GltfType::GLB)
			{
				return ctx.parser.loadGltfBinary(&ctx.data, ctx.parentPath, ctx.options);
			}

			return fastgltf::Expected<fastgltf::Asset>{fastgltf::Error::InvalidGltf};
		};

		fastgltf::GltfType type = determineGltfFileType(&data);
		fastgltf::Expected<fastgltf::Asset> result = loadAsset(context, type);

		if (result)
		{
			return std::move(result.get());
		}


		LogError("Failed to load GLTF: " + std::to_string(fastgltf::to_underlying(result.error())));
		return std::nullopt;
	}


	void CreateMaterials(const fastgltf::Asset &gltf, VulkanContext *vulkanContext, std::vector<std::string> &createdMaterialNames)
	{
		createdMaterialNames.reserve(gltf.materials.size());

		// Load Textures
		std::vector<std::variant<std::filesystem::path, ImageInMemory>> images;

		for (const fastgltf::Image &texture : gltf.images)
		{
			fastgltf::DataSource dataa = texture.data;
			if (const auto uri = std::get_if<fastgltf::sources::URI>(&dataa))
			{
				images.emplace_back(VulkanContext::GetAssetPath() / std::string(uri->uri.string()));
			}

			//TODO: Fix this
			else if (auto array = std::get_if<fastgltf::sources::Array>(&dataa))
			{
				LogError("Array not supported yet");
				// ImageInMemory loadedImage{};
				// if(array->mimeType == fastgltf::MimeType::KTX2)
				// {
				//     auto pD = array->bytes;
				//     ktxTexture* texturePtr{};
				//     auto sources = ktx::CreateImageFromMemory(array->bytes.data(), array->bytes.size(),loadedImage.imageSize, loadedImage.mipLevels, &texturePtr);
				//
				//     loadedImage.texture = texturePtr;
				//     loadedImage.staginBuffer = sources.first;
				//     loadedImage.stagingBufferMemory = sources.second;
				//
				// }else if(array->mimeType == fastgltf::MimeType::PNG || array->mimeType == fastgltf::MimeType::JPEG ||array->mimeType == fastgltf::MimeType::None)
				// {
				//     auto sources = stbi::CreateImageFromMemory(array->bytes.data(), array->bytes.size(), loadedImage.imageSize, loadedImage.mipLevels);
				//     loadedImage.staginBuffer = sources.first;
				//      loadedImage.stagingBufferMemory = sources.second;
				// }else {
				//     LogError("Unsuported embeded texture format in gtlf");
				// }
				//
				// images.emplace_back(loadedImage);

			}

			else if (auto customBuffer = std::get_if<fastgltf::sources::CustomBuffer>(&dataa))
			{
				LogError("CustomBuffer not supported yet");
			}
			else if (auto byteView = std::get_if<fastgltf::sources::ByteView>(&dataa))
			{
				LogError("ByteView not supported yet");
			}
			else if (auto fallback = std::get_if<fastgltf::sources::Fallback>(&dataa))
			{
				LogError("Fallback not supported yet");
			}
			else if (auto bufferView = std::get_if<fastgltf::sources::BufferView>(&dataa))
			{
				LogError("BufferView not supported yet");
			}
		}


		for (const fastgltf::Material &mat : gltf.materials)
		{
			auto newMaterial = MaterialManager::CreateMaterial(vulkanContext, "shader.vert", "PBR_Graypacked.frag", mat.name.c_str());
			createdMaterialNames.emplace_back(mat.name.c_str());

			auto *ubo = newMaterial->GetDescriptorSet()->AddBuffer(0, DescriptorType::UniformBuffer);
			ubo->AddVariable(glm::vec4{1});
			ubo->AddVariable(glm::vec4{1});

			// Load Albedo
			if (mat.pbrData.baseColorTexture.has_value())
			{
				size_t img = gltf.textures[mat.pbrData.baseColorTexture.value().textureIndex].imageIndex.value();
				newMaterial->GetDescriptorSet()->AddTexture(1, images[img], vulkanContext, ColorType::SRGB);
			}
			else
			{
				newMaterial->GetDescriptorSet()->AddTexture(1, "white.ktx", vulkanContext, ColorType::LINEAR);
			}

			// Load Normal
			if (mat.normalTexture.has_value())
			{
				size_t img = gltf.textures[mat.normalTexture.value().textureIndex].imageIndex.value();
				newMaterial->GetDescriptorSet()->AddTexture(2, images[img], vulkanContext, ColorType::LINEAR);
			}
			else
			{
				newMaterial->GetDescriptorSet()->AddTexture(2, "white.ktx", vulkanContext, ColorType::LINEAR);
			}

			// Load graypacked metal/roughness
			if (mat.pbrData.metallicRoughnessTexture.has_value())
			{
				size_t img = gltf.textures[mat.pbrData.metallicRoughnessTexture.value().textureIndex].imageIndex.value();

				newMaterial->GetDescriptorSet()->AddTexture(3, images[img], vulkanContext, ColorType::LINEAR);
			}
			else
			{
				newMaterial->GetDescriptorSet()->AddTexture(3, "white.ktx", vulkanContext, ColorType::LINEAR);
			}

			newMaterial->GetDescriptorSet()->AddTexture(4, "cubemap_vulkan.ktx", vulkanContext, ColorType::SRGB, TextureType::TEXTURE_CUBE);
			newMaterial->CreatePipeline();
		}
	}


	VkFilter GetVkFilter(fastgltf::Filter filter)
	{
		switch (filter)
		{
		// nearest samplers
		case fastgltf::Filter::Nearest:
		case fastgltf::Filter::NearestMipMapNearest:
		case fastgltf::Filter::NearestMipMapLinear:
			return VK_FILTER_NEAREST;

		// linear samplers
		case fastgltf::Filter::Linear:
		case fastgltf::Filter::LinearMipMapNearest:
		case fastgltf::Filter::LinearMipMapLinear: default:
			return VK_FILTER_LINEAR;
		}
	}

	VkSamplerMipmapMode GetVkSamplerMipmapMode(fastgltf::Filter mode)
	{
		switch (mode)
		{
		case fastgltf::Filter::NearestMipMapNearest:
		case fastgltf::Filter::LinearMipMapNearest:
			return VK_SAMPLER_MIPMAP_MODE_NEAREST;

		case fastgltf::Filter::NearestMipMapLinear:
		case fastgltf::Filter::LinearMipMapLinear: default:
			return VK_SAMPLER_MIPMAP_MODE_LINEAR;
		}
	}


} // namespace GLTFLoader


// TODO Move to a seperate thread
namespace ObjLoader
{
	void LoadObj(const std::string &filePath, std::vector<Vertex> &vertices, std::vector<uint32_t> &indices)
	{
		const auto path = VulkanContext::GetAssetPath() / filePath;


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
		for (auto &shape : shapes)
		{
			tinyobj::mesh_t &mesh = shape.mesh;

			for (auto i : mesh.indices)
			{
				glm::vec3 position = {attributes.vertices[3 * i.vertex_index], attributes.vertices[3 * i.vertex_index + 1], attributes.vertices[3 * i.vertex_index + 2]};

				glm::vec3 normal{};
				if (i.normal_index != -1)
				{
					if (3 * i.normal_index + 2 >= attributes.vertices.size())
					{
						LogError("This is not a supported .Obj file! The Normals Index is out of bounds!");
						return;
					}

					normal = {attributes.vertices[3 * i.normal_index], attributes.vertices[3 * i.normal_index + 1], attributes.vertices[3 * i.normal_index + 2]};
				}
				else
				{
					LogInfo("Normal Index Out of bounds!");
				}

				glm::vec2 texCoord{};
				if (i.texcoord_index != -1)
				{
					texCoord = {attributes.texcoords[2 * i.texcoord_index + 0], 1.0f - attributes.texcoords[2 * i.texcoord_index + 1]};
				}
				else
				{
					LogInfo("Texcoord Index Out of bounds!");
				}


				const Vertex vertex = {position, normal, {}, texCoord};

				if (!uniqueVertices.contains(vertex))
				{
					uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
					vertices.emplace_back(vertex);
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
