#pragma once
#include <vulkan/vulkan.h>


//for ease of use in the future we will track the state as otherwise is only tracked internally by Vulkan
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

	static void CreateCommandBuffer(const VkDevice& device, const VkCommandPool& commandPool, CommandBuffer& commandBuffer, bool isPrimary = true);
	static void FreeCommandBuffer(const VkDevice& device, const VkCommandPool& commandPool, CommandBuffer& commandBuffer);

	static void BeginCommandBufferRecording(CommandBuffer& commandBuffer, bool isRenderpassContinue, bool isSimultaneous, bool isSingeUse = false);
	static void EndCommandBufferRecording(CommandBuffer& commandBuffer);

	static void SubmitCommandBuffer(CommandBuffer& commandBuffer);
	static void ResetCommandBuffer(const VkDevice& device, const VkCommandPool& commandPool, CommandBuffer& commandBuffer);

	static void CreateCommandBufferSingleUse(const VkDevice& device, const VkCommandPool& commandPool, CommandBuffer& commandBuffer);
	static void EndCommandBufferSingleUse(const VkDevice& device, const VkCommandPool& commandPool, CommandBuffer& commandBuffer, VkQueue queue);
};

