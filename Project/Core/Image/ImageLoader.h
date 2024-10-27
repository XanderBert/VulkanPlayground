#pragma once
#include <ktxvulkan.h>
#include <optional>

#include "Texture.h"
#include "vk_mem_alloc.h"
#include "vulkanbase/VulkanTypes.h"


enum class TextureType : uint8_t;

namespace Image
{
    void CreateImage(uint32_t width, uint32_t height, uint32_t mipLevels,
        VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
        VkImage& image, VmaAllocation& imageMemory, TextureType textureType);

    void CreateImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, VkImageView& imageView, TextureType textureType);
    void CreateSampler(const VulkanContext * vulkanContext, VkSampler& sampler, uint32_t mipLevels,const std::optional<VkSamplerCreateInfo> &overridenSamplerInfo = std::nullopt);

    bool HasStencilComponent(VkFormat format);
}


//stbi loader
namespace stbi
{
    std::pair<VkBuffer, VmaAllocation> CreateImage(const std::filesystem::path &path, glm::ivec2 &imageSize, uint32_t &mipLevels);
    std::pair<VkBuffer, VmaAllocation> CreateImageFromMemory(const std::uint8_t* data, size_t size, glm::ivec2 &imageSize, uint32_t &mipLevels);
}

//ktx Loader
namespace ktx
{
    ktxVulkanTexture CreateImage(const std::filesystem::path &path);
    std::pair<VkBuffer, VmaAllocation> CreateImageFromMemory(const std::uint8_t* data, size_t size, glm::ivec2 &imageSize, uint32_t &mipLevels, ktxTexture** texture);
}