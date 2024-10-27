#pragma once
#include <memory>
#include <vulkan/vulkan.h>
#include "Core/VmaUsage.h"


class ImGuiTexture;

class ColorAttachment
{
public:
	ColorAttachment() = default;
	~ColorAttachment() = default;
	ColorAttachment(const ColorAttachment&) = delete;
	ColorAttachment& operator=(const ColorAttachment&) = delete;
	ColorAttachment(ColorAttachment&&) = delete;
	ColorAttachment& operator=(ColorAttachment&&) = delete;

	void Init(const VulkanContext *vulkanContext, VkFormat format, VkClearColorValue clearColor);
	void Cleanup(const VulkanContext* vulkanContext);

	[[nodiscard]] VkRenderingAttachmentInfoKHR* GetRenderingAttachmentInfo();

	void ResetImageLayout();
	void TransitionToWrite(VkCommandBuffer commandBuffer);
	void TransitionToRead(VkCommandBuffer commandBuffer);

	void OnImGui();
private:

	void Setup(const VulkanContext *vulkanContext, VkClearColorValue clearColor);
	void Recreate(const VulkanContext *vulkanContext, VkClearColorValue clearColor);

	VkRenderingAttachmentInfoKHR m_ColorAttachmentInfo;

	VkImage m_Image;
	VmaAllocation m_Memory;
	VkImageView m_ImageView;
	VkFormat m_Format;
	VkSampler m_Sampler{VK_NULL_HANDLE};

	VkImageLayout m_CurrentImageLayout{VK_IMAGE_LAYOUT_UNDEFINED};

	std::unique_ptr<ImGuiTexture> m_DebugTexture{};
	bool m_RecreateDebugTexture{false};
};
