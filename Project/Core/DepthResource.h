#pragma once
#include <vulkan/vulkan.h>

#include "Core/VmaUsage.h"
#include "vulkanbase/VulkanTypes.h"

class ImGuiTexture;

class DepthResource
{
public:

	static void Recreate(const VulkanContext* vulkanContext);
	static void Init(const VulkanContext* vulkanContext);
	static void Cleanup(const VulkanContext* vulkanContext);

	static  VkPipelineDepthStencilStateCreateInfo GetDepthPipelineInfo(VkBool32 depthTestEnable, VkBool32 depthWriteEnable);

	static VkImageView GetImageView();
	static VkFormat GetFormat();
	static VkImage GetImage();

    static void DebugRenderDepthResource(const VulkanContext* vulkanContext);

private:
	inline static VkImage m_Image;
	inline static VmaAllocation m_Memory;
	inline static VkImageView m_ImageView;
	inline static VkFormat m_Format;


    inline static VkSampler m_Sampler{VK_NULL_HANDLE};
    inline static std::unique_ptr<ImGuiTexture> m_ImGuiTexture{};
};

class DepthResourceBuilder
{
public:
    static void Build(const VulkanContext* vulkanContext, VkImage& image, VkImageView& imageView , VmaAllocation& memory, VkFormat& format);
private:
    static VkFormat FindDepthFormat(const VulkanContext* vulkanContext);
    static  void CreateDepthResources(const VulkanContext* vulkanContext, VkImage& image, VkImageView& imageView , VmaAllocation& memory);
    static VkFormat FindSupportedFormat(const VulkanContext* vulkanContext, const std::vector<VkFormat>& candidates, bool isDepthOnly);
};