#pragma once
#include <vulkan/vulkan.h>
#include <vector>

#include "Core/VmaUsage.h"
#include "vulkanbase/VulkanTypes.h"

namespace Descriptor
{
	class DescriptorWriter;
}

class ImGuiTexture;

class DepthAttachment final
{
public:

	DepthAttachment() = default;
	~DepthAttachment() = default;
	DepthAttachment(const DepthAttachment&) = delete;
	DepthAttachment& operator=(const DepthAttachment&) = delete;
	DepthAttachment(DepthAttachment&&) = delete;
	DepthAttachment& operator=(DepthAttachment&&) = delete;

	void Init(const VulkanContext* vulkanContext);
	void Cleanup(const VulkanContext* vulkanContext);
	void Bind(Descriptor::DescriptorWriter& writer, int bindingNumber) const;

	[[nodiscard]] VkRenderingAttachmentInfoKHR* GetRenderingAttachmentInfo();
	[[nodiscard]] static VkPipelineDepthStencilStateCreateInfo GetDepthPipelineInfo(VkBool32 depthTestEnable, VkBool32 depthWriteEnable);
	[[nodiscard]] VkImageLayout GetBindImageLayout();
	[[nodiscard]] VkFormat GetFormat() const;

    void OnImGui();

    void TransitionToDepthResource(VkCommandBuffer commandBuffer);
    void TransitionToShaderRead(VkCommandBuffer commandBuffer);
    void TransitionToGeneralResource(VkCommandBuffer commandBuffer);
    void ResetImageLayout();

private:
	void Recreate(const VulkanContext* vulkanContext);

	VkImage m_Image{};
	VmaAllocation m_Memory{};
	VkImageView m_ImageView{};
	VkFormat m_Format{};
    VkSampler m_Sampler{VK_NULL_HANDLE};

    std::unique_ptr<ImGuiTexture> m_ImGuiTexture{};
    VkImageLayout m_BindImageLayout{VK_IMAGE_LAYOUT_UNDEFINED};

	VkRenderingAttachmentInfoKHR m_DepthAttachmentInfo{};
};

class DepthResourceBuilder
{
public:
    static void Build(const VulkanContext* vulkanContext, VkImage& image, VkImageView& imageView , VmaAllocation& memory, VkFormat& format);
private:
    static VkFormat FindDepthFormat(const VulkanContext* vulkanContext);
    static  void CreateDepthResources(const VulkanContext* vulkanContext, VkImage& image, VkImageView& imageView , VmaAllocation& memory, const VkFormat& format);
    static VkFormat FindSupportedFormat(const VulkanContext* vulkanContext, const std::vector<VkFormat>& candidates, bool isDepthOnly);
};