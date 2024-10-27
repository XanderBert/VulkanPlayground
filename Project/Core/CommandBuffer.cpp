#include "CommandBuffer.h"
#include <stdexcept>

#include "Logger.h"
#include "vulkanbase/VulkanTypes.h"


void CommandBufferManager::CreateCommandBuffer(const VulkanContext* vulkanContext, CommandBuffer& commandBuffer, bool isPrimary )
{
	//reset the command buffer
	commandBuffer.Handle = VK_NULL_HANDLE;
	
	const auto level = isPrimary ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY;
	const VkCommandBufferAllocateInfo allocInfo
	{
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		nullptr,
		vulkanContext->commandPool,
		level,
		1
	};

	commandBuffer.State = CommandBufferState::NotAllocated;

	VulkanCheck(vkAllocateCommandBuffers(vulkanContext->device, &allocInfo, &commandBuffer.Handle), "failed to allocate command buffers!")

	commandBuffer.State = CommandBufferState::Ready;
}

void CommandBufferManager::FreeCommandBuffer(const VkDevice& device, const VkCommandPool& commandPool, CommandBuffer& commandBuffer)
{
		vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer.Handle);
		commandBuffer.Handle = VK_NULL_HANDLE;
		commandBuffer.State = CommandBufferState::NotAllocated;
}

void CommandBufferManager::BeginCommandBufferRecording(CommandBuffer& commandBuffer, bool isRenderpassContinue, bool isSimultaneous, bool isSingeUse)
{
	LogAssert(commandBuffer.State == CommandBufferState::Ready, "Command buffer not allocated", true)

	VkCommandBufferUsageFlags flags = 0;
	flags |= isSingeUse ? VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT : 0;
	flags |= isRenderpassContinue ? VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT : 0;
	flags |= isSimultaneous ? VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT : 0;


	const VkCommandBufferBeginInfo beginInfo
	{
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		nullptr,
		flags,
		nullptr
	};

	VulkanCheck(vkBeginCommandBuffer(commandBuffer.Handle, &beginInfo), "failed to begin recording command buffer!")

	commandBuffer.State = CommandBufferState::Recording;
}

void CommandBufferManager::EndCommandBufferRecording(CommandBuffer& commandBuffer)
{
	LogAssert(commandBuffer.State == CommandBufferState::Recording, "Command buffer not recording", true)
	VulkanCheck(vkEndCommandBuffer(commandBuffer.Handle), "failed to record command buffer!")

	commandBuffer.State = CommandBufferState::RecordingEnded;
}

void CommandBufferManager::SubmitCommandBuffer(const VulkanContext* vulkanContext, CommandBuffer& commandBuffer,const VkSubmitInfo* submitInfo, VkFence fence)
{
	LogAssert(commandBuffer.State == CommandBufferState::RecordingEnded, "Command buffer not ready for submission", true)
	commandBuffer.State = CommandBufferState::Submitted;
	
	VulkanCheck(vkQueueSubmit(vulkanContext->graphicsQueue, 1, submitInfo, fence), "Failed To Submit Queue.");
}

void CommandBufferManager::ResetCommandBuffer(CommandBuffer& commandBuffer)
{
	vkResetCommandBuffer(commandBuffer.Handle, 0);
	commandBuffer.State = CommandBufferState::Ready;
}

void CommandBufferManager::CreateCommandBufferSingleUse(const VulkanContext* vulkanContext, CommandBuffer& commandBuffer)
{
	CreateCommandBuffer(vulkanContext, commandBuffer);
	BeginCommandBufferRecording(commandBuffer, false, false, true);
}

void CommandBufferManager::EndCommandBufferSingleUse(const VulkanContext* vulkanContext, CommandBuffer& commandBuffer)
{
	EndCommandBufferRecording(commandBuffer);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer.Handle;


	VulkanCheck(vkQueueSubmit(vulkanContext->graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE), "Failed to submet Draw Command Buffer")

	//TODO
	//there is not enough overhead to justify using fences for single use command buffers (for now at least)
	vkQueueWaitIdle(vulkanContext->graphicsQueue);

	FreeCommandBuffer(vulkanContext->device, vulkanContext->commandPool, commandBuffer);
}