#include "ModelLoader.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#include "Vertex.h"
#include "Core/Logger.h"


namespace ObjLoader
{
	void LoadObj(const std::string& filePath, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices)
	{
		tinyobj::attrib_t attributes;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string errors;

		const bool success = tinyobj::LoadObj(&attributes, &shapes, &materials, &errors, filePath.c_str());

		if (!success)
			LogWarning("Failed to load: " + filePath);

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

				glm::vec3 normal =
				{
					attributes.vertices[3 * i.vertex_index],
					attributes.vertices[3 * i.vertex_index + 1],
					attributes.vertices[3 * i.vertex_index + 2]
				};

				glm::vec2 texCoord =
				{
					attributes.texcoords[2 * i.texcoord_index + 0],
					1.0f - attributes.texcoords[2 * i.texcoord_index + 1]
				};


				const Vertex vert = { position, normal, texCoord };
				vertices.push_back(vert);
				indices.push_back(indices.size());
			}
		}
	}
}
