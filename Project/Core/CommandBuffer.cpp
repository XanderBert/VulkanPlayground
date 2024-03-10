#include "CommandBuffer.h"
#include <stdexcept>


void CommandBufferManager::CreateCommandBuffer(const VkDevice& device, const VkCommandPool& commandPool,CommandBuffer& commandBuffer, bool isPrimary )
{
	//reset the command buffer
	commandBuffer.Handle = VK_NULL_HANDLE;
	
	const auto level = isPrimary ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY;
	const VkCommandBufferAllocateInfo allocInfo
	{
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		nullptr,
		commandPool,
		level,
		1
	};

	commandBuffer.State = CommandBufferState::NotAllocated;

	if (vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer.Handle) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate command buffers!");
	}

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
	if (commandBuffer.State != CommandBufferState::Ready)
	{
		throw std::runtime_error("Command buffer not ready for recording");
	}


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

	if (vkBeginCommandBuffer(commandBuffer.Handle, &beginInfo) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to begin recording command buffer!");
	}

	commandBuffer.State = CommandBufferState::Recording;
}

void CommandBufferManager::EndCommandBufferRecording(CommandBuffer& commandBuffer)
{
	if (commandBuffer.State != CommandBufferState::Recording)
	{
		throw std::runtime_error("Command buffer not ready for ending recording");
	}

	if (vkEndCommandBuffer(commandBuffer.Handle) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to record command buffer!");
	}

	commandBuffer.State = CommandBufferState::RecordingEnded;
}

void CommandBufferManager::SubmitCommandBuffer(CommandBuffer& commandBuffer)
{
	if (commandBuffer.State != CommandBufferState::RecordingEnded)
	{
		throw std::runtime_error("Command buffer not ready for submission");
	}



	commandBuffer.State = CommandBufferState::Submitted;
}

void CommandBufferManager::ResetCommandBuffer(const VkDevice& device, const VkCommandPool& commandPool, CommandBuffer& commandBuffer)
{
	vkResetCommandBuffer(commandBuffer.Handle, 0);
	commandBuffer.State = CommandBufferState::Ready;
}

void CommandBufferManager::CreateCommandBufferSingleUse(const VkDevice& device, const VkCommandPool& commandPool, CommandBuffer& commandBuffer)
{
	CreateCommandBuffer(device, commandPool, commandBuffer);
	BeginCommandBufferRecording(commandBuffer, true, false);
}

void CommandBufferManager::EndCommandBufferSingleUse(const VkDevice& device, const VkCommandPool& commandPool, CommandBuffer& commandBuffer, VkQueue queue)
{
	EndCommandBufferRecording(commandBuffer);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer.Handle;

	if (vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to submit draw command buffer!");
	}

	//there is not enough overhead to justify using fences for single use command buffers (for now at least)
	vkQueueWaitIdle(queue);

	FreeCommandBuffer(device, commandPool, commandBuffer);
}
