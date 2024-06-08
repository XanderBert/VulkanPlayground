#pragma once
#include <vulkan/vulkan.h>
#include "vulkanbase/VulkanTypes.h"
#include "Core/VmaUsage.h"

struct Vertex;
namespace Core
{
	namespace Buffer
	{
	    void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkBuffer& buffer, VmaAllocation& bufferMemory, bool willBeMapped = false, bool willBePersistantmapped = false);

		template <typename T>
		void CreateStagingBuffer(VkDeviceSize size, VkBuffer& buffer, VmaAllocation& bufferMemory, const T* actualData)
		{
			CreateBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, buffer, bufferMemory, true);

			//Copy the data to the staging buffer
		    void* data;
		    vmaMapMemory(Allocator::VmaAllocator, bufferMemory, &data);
			memcpy(data, actualData, size);
		    vmaUnmapMemory(Allocator::VmaAllocator, bufferMemory);
		}

		void CopyBuffer(VulkanContext* vulkanContext, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

	}
}