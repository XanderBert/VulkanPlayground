#pragma once
#include <vulkan/vulkan.h>

class RenderPass
{
public:
	RenderPass() = default;
	~RenderPass() = default;
	RenderPass(const RenderPass&) = delete;
	RenderPass& operator=(const RenderPass&) = delete;
	RenderPass(RenderPass&&) = delete;
	RenderPass& operator=(RenderPass&&) = delete;

	static void CreateRenderPass(const VkDevice& device, const VkFormat& swapChainImageFormat, VkRenderPass& renderPass);

};