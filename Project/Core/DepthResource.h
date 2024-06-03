#pragma once
#include <vulkan/vulkan.h>
#include "vulkanbase/VulkanTypes.h"

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

private:
	inline static VkImage m_Image;
	inline static VkDeviceMemory m_Memory;
	inline static VkImageView m_ImageView;
	inline static VkFormat m_Format;
};

class DepthResourceBuilder
{
public:
    static void Build(const VulkanContext* vulkanContext, VkImage& image, VkImageView& imageView , VkDeviceMemory& memory, VkFormat& format);
private:
    static VkFormat FindDepthFormat(const VulkanContext* vulkanContext);
    static  void CreateDepthResources(const VulkanContext* vulkanContext, VkImage& image, VkImageView& imageView , VkDeviceMemory& memory);
    static VkFormat FindSupportedFormat(const VulkanContext* vulkanContext, const std::vector<VkFormat>& candidates, bool isDepthOnly);
};