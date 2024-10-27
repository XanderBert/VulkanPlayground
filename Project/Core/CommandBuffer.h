#pragma once
#include <vulkan/vulkan.h>

class VulkanContext;

enum class CommandBufferState : uint8_t
{
	NotAllocated,
	Ready,
	Recording,
	RecordingEnded,
	InRenderPass,
	Submitted,
};

struct CommandBuffer
{
	VkCommandBuffer Handle;
	CommandBufferState State;
};


class CommandBufferManager
{
public:
	CommandBufferManager() = default;
	~CommandBufferManager() = default;

	CommandBufferManager(const CommandBufferManager&) = delete;
	CommandBufferManager(CommandBufferManager&&) = delete;
	CommandBufferManager& operator=(const CommandBufferManager&) = delete;
	CommandBufferManager& operator=(CommandBufferManager&&) = delete;

	static void CreateCommandBuffer(const VulkanContext* vulkanContext, CommandBuffer& commandBuffer, bool isPrimary = true);
	static void FreeCommandBuffer(const VkDevice& device, const VkCommandPool& commandPool, CommandBuffer& commandBuffer);

	static void BeginCommandBufferRecording(CommandBuffer& commandBuffer, bool isRenderpassContinue, bool isSimultaneous, bool isSingeUse = false);
	static void EndCommandBufferRecording(CommandBuffer& commandBuffer);

	static void SubmitCommandBuffer(const VulkanContext* vulkanContext, CommandBuffer& commandBuffer,const VkSubmitInfo* submitInfo, VkFence fence);
	static void ResetCommandBuffer(CommandBuffer& commandBuffer);

	static void CreateCommandBufferSingleUse(const VulkanContext* vulkanContext, CommandBuffer& commandBuffer);
	static void EndCommandBufferSingleUse(const VulkanContext* vulkanContext, CommandBuffer& commandBuffer);
};

