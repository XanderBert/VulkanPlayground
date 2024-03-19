#include "Buffer.h"

#include "Logger.h"
#include "Mesh/Vertex.h"
#include "vulkanbase/VulkanBase.h"

namespace Core
{
	namespace Buffer
	{
		void CreateBuffer(VulkanContext* vulkanContext, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
		{
			VkBufferCreateInfo bufferInfo{};
			bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			bufferInfo.size = size;
			bufferInfo.usage = usage;
			bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;


			const VkDevice device = vulkanContext->device;

			VulkanCheck(vkCreateBuffer(device, &bufferInfo, nullptr, &buffer), "Failed to create buffer!");


			VkMemoryRequirements memRequirements;
			vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

			VkMemoryAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			allocInfo.allocationSize = memRequirements.size;
			allocInfo.memoryTypeIndex = tools::findMemoryType(memRequirements.memoryTypeBits, properties, vulkanContext->physicalDevice);


			VulkanCheck(vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory), "Failed to allocate buffer memory!")
			

			vkBindBufferMemory(device, buffer, bufferMemory, 0);
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