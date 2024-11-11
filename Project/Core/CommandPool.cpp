#include "CommandPool.h"
#include "Logger.h"
#include "QueueFamilyIndices.h"
#include "SwapChain.h"


namespace CommandPool
{
	void CreateCommandPool(VulkanContext* context)
	{
		const QueueFamilyIndices queueFamilyIndices = QueueFamilyIndices::FindQueueFamilies(context->physicalDevice, SwapChain::GetSurface());


		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

		VulkanCheck(vkCreateCommandPool(context->device, &poolInfo, nullptr, &context->commandPool), "Failed To Create ComandPool")
	}
} // namespace CommandPool