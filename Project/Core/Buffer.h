#pragma once
#include <vulkan/vulkan.h>
#include "vulkanbase/VulkanTypes.h"

struct Vertex;
namespace Core
{
	namespace Buffer
	{
		void CreateBuffer(VulkanContext* vulkanContext, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);

		template <typename T>
		inline void CreateStagingBuffer(VulkanContext* vulkanContext, VkDeviceSize size, VkBuffer& buffer, VkDeviceMemory& bufferMemory, const T* actualData)
		{
			CreateBuffer(vulkanContext, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, buffer, bufferMemory);

			//Copy the vertex data to the staging buffer
			void* data;
			vkMapMemory(vulkanContext->device, bufferMemory, 0, size, 0, &data);
			memcpy(data, actualData, size);
			vkUnmapMemory(vulkanContext->device, bufferMemory);
		};

		void CopyBuffer(VulkanContext* vulkanContext, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

	}
}