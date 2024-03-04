#pragma once
#include <vulkan/vulkan.h>
#include "vulkanbase/VulkanUtil.h"

class CommandPool final
{
	public:
	CommandPool() = default;
	~CommandPool() = default;
	CommandPool(const CommandPool&) = delete;
	CommandPool& operator=(const CommandPool&) = delete;
	CommandPool(CommandPool&&) = delete;
	CommandPool& operator=(CommandPool&&) = delete;

	static void CreateCommandPool(const VkDevice& device, const VkPhysicalDevice& physicalDevice, const VkSurfaceKHR& surface, VkCommandPool& commandPool);
};