#pragma once
#include <vulkan/vulkan.h>

#include "vulkanbase/VulkanTypes.h"


class CommandPool final
{
	public:
	CommandPool() = default;
	~CommandPool() = default;
	CommandPool(const CommandPool&) = delete;
	CommandPool& operator=(const CommandPool&) = delete;
	CommandPool(CommandPool&&) = delete;
	CommandPool& operator=(CommandPool&&) = delete;

	static void CreateCommandPool(const VulkanContext* context, VkCommandPool& commandPool);
};