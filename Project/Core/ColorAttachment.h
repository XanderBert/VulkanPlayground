#pragma once
#include <memory>
#include <vulkan/vulkan.h>
#include "Core/VmaUsage.h"
#include "glm/vec2.hpp"


namespace Descriptor
{
	class DescriptorWriter;
}

class ImGuiTexture;

class ColorAttachment final
{
public:
	ColorAttachment() = default;
	~ColorAttachment() = default;
	ColorAttachment(const ColorAttachment&) = delete;
	ColorAttachment& operator=(const ColorAttachment&) = delete;
	ColorAttachment(ColorAttachment&&) = delete;
	ColorAttachment& operator=(ColorAttachment&&) = delete;

	void Init(const VulkanContext *vulkanContext, VkFormat format, VkClearColorValue clearColor,const glm::ivec2& extent);
	void Recreate(const VulkanContext *vulkanContext, VkClearColorValue clearColor, const glm::ivec2& extent);

	void Cleanup(VkDevice device);
	void Bind(Descriptor::DescriptorWriter& writer, int bindingNumber) const;

	[[nodiscard]] VkRenderingAttachmentInfoKHR* GetRenderingAttachmentInfo();
	[[nodiscard]] VkFormat* GetFormat();
	[[nodiscard]] VkImage GetImage() const;

	void ResetImageLayout();
	void TransitionToWrite(VkCommandBuffer commandBuffer);
	void TransitionToRead(VkCommandBuffer commandBuffer);
	void TransitionToGeneralResource(VkCommandBuffer commandBuffer);


	void OnImGui();
private:
	void Setup(const VulkanContext *vulkanContext, VkClearColorValue clearColor, const glm::ivec2& extent);


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
