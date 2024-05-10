#include "ModelLoader.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#include <unordered_map>

#include "Vertex.h"
#include "Core/Logger.h"

//TODO Move to a seperate thread
namespace ObjLoader
{
	void LoadObj(const std::string& filePath, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices)
	{
		tinyobj::attrib_t attributes;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string errors;
		std::string warnings;

		const bool success = tinyobj::LoadObj(&attributes, &shapes, &materials, &warnings, &errors, filePath.c_str());


		if (!success)
		{
			LogError("Failed to load: " + filePath);
			LogError(errors);
			LogWarning(warnings);
		    return;
		}

        std::string logMaterials = filePath + " has " + std::to_string(materials.size()) + " Materials";
	    LogInfo(logMaterials);

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
				}

                glm::vec3 tangent{};
			    tangent = glm::cross(position, normal);


				glm::vec2 texCoord{};
				if(i.texcoord_index != -1)
				{
					texCoord =
					{
						attributes.texcoords[2 * i.texcoord_index + 0],
						1.0f - attributes.texcoords[2 * i.texcoord_index + 1]
					};
				}
	


				const Vertex vertex = { position, normal, tangent, texCoord };

				if (!uniqueVertices.contains(vertex))
				{
					uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
					vertices.push_back(vertex);
				}

				indices.push_back(uniqueVertices[vertex]);
			}
		}

	    LogInfo("Successfully loaded: " + filePath);
	}
}
