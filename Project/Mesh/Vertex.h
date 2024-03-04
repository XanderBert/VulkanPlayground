#pragma once

#include <array>
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

struct Vertex 
{
    glm::vec2 pos;
    glm::vec3 color;

    static VkVertexInputBindingDescription GetBindingDescription()
    {
		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);

		//VK_VERTEX_INPUT_RATE_INSTANCE for instanced rendering
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}


	static std::array<VkVertexInputAttributeDescription, 2> GetAttributeDescriptions()
	{
		std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};

		//position
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, pos);

		//color
		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, color);


		return attributeDescriptions;
	}
};



