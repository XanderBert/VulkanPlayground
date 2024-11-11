#include "Buffer.h"
#include "Logger.h"
#include "vulkanbase/VulkanBase.h"

#include "VmaUsage.h"

namespace Core
{
	namespace Buffer
	{
		void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkBuffer& buffer, VmaAllocation& bufferMemory, bool willBeMapped, bool willBePersistantmapped)
		{
			VkBufferCreateInfo bufferInfo{};
			bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			bufferInfo.size = size;
			bufferInfo.usage = usage;
			bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		    VmaAllocationCreateInfo allocInfo = {};
		    allocInfo.usage = VMA_MEMORY_USAGE_AUTO;

		    if(willBeMapped)
		    {
		        allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
		    }

		    if(willBePersistantmapped)
		    {
		        allocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
		    }


		    VulkanCheck(vmaCreateBuffer(Allocator::vmaAllocator, &bufferInfo, &allocInfo, &buffer, &bufferMemory, nullptr), "Failed to create Vma Buffer");
		}

		

		void CopyBuffer(VulkanContext* vulkanContext, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
		{
			CommandBuffer commandBuffer;
			CommandBufferManager::CreateCommandBufferSingleUse(vulkanContext, commandBuffer);

			VkBufferCopy copyRegion{};
			copyRegion.size = size;

			vkCmdCopyBuffer(commandBuffer.Handle, srcBuffer, dstBuffer, 1, &copyRegion);
			CommandBufferManager::EndCommandBufferSingleUse(vulkanContext, commandBuffer);
		}		
	} // namespace Buffer
} // namespace Core