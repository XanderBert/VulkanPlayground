#pragma once
#include <optional>
#include <vulkan/vulkan_core.h>
#include "vulkanbase/VulkanTypes.h"


namespace Image {
    void CreateImage(
        const VulkanContext* vulkanContext,
        uint32_t width,
        uint32_t height,
        uint32_t mipLevels,
        VkSampleCountFlagBits numSamples,
        VkFormat format,
        VkImageTiling tiling,
        VkImageUsageFlags usage,
        VkMemoryPropertyFlags properties,
        VkImage& image,
        VkDeviceMemory& imageMemory,
        bool isCubeMap = false);

    void CreateImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, VkImageView& imageView);
    void CreateCubeImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, VkImageView& imageView);
    void CreateSampler(const VulkanContext * vulkanContext, VkSampler& sampler, uint32_t mipLevels,const std::optional<VkSamplerCreateInfo> &overridenSamplerInfo = std::nullopt);

    bool HasStencilComponent(VkFormat format);
}



