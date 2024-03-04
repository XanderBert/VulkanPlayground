#include <array>

#include "commandPool.h"
#include "QueueFamilyIndices.h"

void CommandPool::CreateCommandPool(const VkDevice& device, const VkPhysicalDevice& physicalDevice,
	const VkSurfaceKHR& surface, VkCommandPool& commandPool)
{
	const QueueFamilyIndices queueFamilyIndices = QueueFamilyIndices::FindQueueFamilies(physicalDevice, surface);


	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

	if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create command pool!");
	}
}