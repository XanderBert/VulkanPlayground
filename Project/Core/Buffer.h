#pragma once
#include <vulkan/vulkan.h>
#include <cstring>
#include "vulkanbase/VulkanTypes.h"
#include "Core/VmaUsage.h"

struct Vertex;

struct Buffer
{
	int count;
	VkBuffer buffer;
	VmaAllocation bufferMemory;

	inline void BindAsIndexBuffer(VkCommandBuffer commandBuffer) const
	{
		vkCmdBindIndexBuffer(commandBuffer, buffer, 0, VK_INDEX_TYPE_UINT32);
	}

	inline void BindAsVertexBuffer(VkCommandBuffer commandBuffer) const
	{
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, &buffer, offsets);
	}

	inline void Cleanup() const
	{
		vmaDestroyBuffer(Allocator::vmaAllocator, buffer, bufferMemory);
	}
};


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
		    vmaMapMemory(Allocator::vmaAllocator, bufferMemory, &data);
			memcpy(data, actualData, size);
		    vmaUnmapMemory(Allocator::vmaAllocator, bufferMemory);
		}

		void CopyBuffer(VulkanContext* vulkanContext, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	}
}