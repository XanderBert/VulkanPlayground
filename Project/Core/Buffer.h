#pragma once
#include <vulkan/vulkan.h>
#include "vulkanbase/VulkanTypes.h"

struct Vertex;
namespace Core
{
	namespace Buffer
	{
		void CreateBuffer(VulkanContext* vulkanContext, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
		void CreateStagingBuffer(VulkanContext* vulkanContext, VkDeviceSize size, const std::vector<Vertex>& vertices, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
		void CopyBuffer(VulkanContext* vulkanContext, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);


	}
}