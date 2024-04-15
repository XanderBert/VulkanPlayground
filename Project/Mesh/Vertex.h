#pragma once

#include <array>
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

struct Vertex 
{
    glm::vec3 pos;
    glm::vec3 normal;
	glm::vec2 texCoord;

	bool operator==(const Vertex& other) const
	{
		return pos == other.pos && normal == other.normal && texCoord == other.texCoord;
	}

    static VkVertexInputBindingDescription GetBindingDescription()
    {
		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);

		//VK_VERTEX_INPUT_RATE_INSTANCE for instanced rendering
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}


	static std::array<VkVertexInputAttributeDescription, 3> GetAttributeDescriptions()
	{
		std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};

		//position
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, pos);

		//color
		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, normal);

		//texture coordinates
		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[2].offset = offsetof(Vertex, texCoord);


		return attributeDescriptions;
	}
};

struct VertexHasher
{
	size_t operator()(Vertex const& vertex) const
	{
		return ((std::hash<float>()(vertex.pos.x) ^
				((std::hash<float>()(vertex.pos.y) ^
				((std::hash<float>()(vertex.pos.z) ^
			(std::hash<float>()(vertex.normal.x) << 1)) >> 1) ^
			(std::hash<float>()(vertex.normal.y) << 1)) >> 1) ^
			(std::hash<float>()(vertex.normal.z) << 1)) >> 1) ^
			(std::hash<float>()(vertex.texCoord.x) << 1) ^
			(std::hash<float>()(vertex.texCoord.y) << 1);
	}
};
