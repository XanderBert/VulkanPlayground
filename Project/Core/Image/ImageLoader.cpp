#include "ImageLoader.h"
#include <ImGuiFileDialog.h>
#include "Core/CommandBuffer.h"
#include "Core/Logger.h"
#include "vulkanbase/VulkanBase.h"

namespace Image
{
	void CreateImage(const VulkanContext* vulkanContext, uint32_t width, uint32_t height, uint32_t mipLevels,
		VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
		VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory, bool isCubeMap)
	{
		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = width;
		imageInfo.extent.height = height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = mipLevels;
		imageInfo.arrayLayers = 1;
		imageInfo.format = format;
		imageInfo.tiling = tiling;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = usage;
		imageInfo.samples = numSamples;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if(isCubeMap)
        {
            imageInfo.arrayLayers = 6;
            imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
        }


		VulkanCheck(vkCreateImage(vulkanContext->device, &imageInfo, nullptr, &image), "Failed To Create Image")


		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(vulkanContext->device, image, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = tools::findMemoryType(memRequirements.memoryTypeBits, properties, vulkanContext->physicalDevice);

		VulkanCheck(vkAllocateMemory(vulkanContext->device, &allocInfo, nullptr, &imageMemory), "Failed To Allocate memory for the depth image")
		VulkanCheck(vkBindImageMemory(vulkanContext->device, image, imageMemory, 0), "Failed To Bind Image memory")
	}

	void CreateImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, VkImageView &imageView) {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = format;

        viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        viewInfo.subresourceRange.aspectMask = aspectFlags;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        VulkanCheck(vkCreateImageView(device, &viewInfo, nullptr, &imageView), "Failed to create texture image view!")
    }
    void CreateCubeImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags,
                             VkImageView &imageView)
    {
        const VkImageViewCreateInfo viewInfo
	    {
	        VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
	        nullptr,
	        0,
	        image,
	        VK_IMAGE_VIEW_TYPE_CUBE,
	        format,
	        {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY},
	        {aspectFlags, 0, 1, 0, 6}
	    };

        VulkanCheck(vkCreateImageView(device, &viewInfo, nullptr, &imageView), "Failed to create texture image view!")
    }

    void CreateSampler(const VulkanContext* vulkanContext, VkSampler& sampler, uint32_t mipLevels, const std::optional<VkSamplerCreateInfo> &overridenSamplerInfo)
	{
		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(vulkanContext->physicalDevice, &properties);

	    VkSamplerCreateInfo samplerInfo{};
	    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	    samplerInfo.magFilter = VK_FILTER_LINEAR;
	    samplerInfo.minFilter = VK_FILTER_LINEAR;
	    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	    samplerInfo.anisotropyEnable = VK_TRUE;
	    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
	    samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	    samplerInfo.unnormalizedCoordinates = VK_FALSE;
	    samplerInfo.compareEnable = VK_FALSE;
	    samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
	    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	    samplerInfo.mipLodBias = 0.0f;
	    samplerInfo.minLod = 0.0f;
	    samplerInfo.maxLod = static_cast<float>(mipLevels);

        if(overridenSamplerInfo.has_value())
        {
            samplerInfo.magFilter = overridenSamplerInfo.value().magFilter;
            samplerInfo.minFilter = overridenSamplerInfo.value().minFilter;
            samplerInfo.mipmapMode = overridenSamplerInfo.value().mipmapMode;
        }

		VulkanCheck(vkCreateSampler(vulkanContext->device, &samplerInfo, nullptr, &sampler), "Failed to create texture sampler!")
	}

	bool HasStencilComponent(VkFormat format)
	{
		return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
	}
}