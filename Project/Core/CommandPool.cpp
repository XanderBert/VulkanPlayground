#include "commandPool.h"
#include "QueueFamilyIndices.h"


namespace CommandPool
{
	void CreateCommandPool(VulkanContext* context)
	{
		const QueueFamilyIndices queueFamilyIndices = QueueFamilyIndices::FindQueueFamilies(context->physicalDevice, context->surface);


		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();


		if (vkCreateCommandPool(context->device, &poolInfo, nullptr, &context->commandPool) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create command pool!");
		}
	}
} // namespace CommandPool