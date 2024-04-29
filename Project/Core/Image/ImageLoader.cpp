#include "ImageLoader.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include "Core/Buffer.h"
#include "Core/CommandBuffer.h"
#include "Core/Descriptor.h"
#include "Core/Logger.h"
#include "vulkanbase/VulkanBase.h"

namespace ImageLoader
{
	void CreateTexture(const std::string& path, VulkanContext* vulkanContext, VkImage& image, VkDeviceMemory& imageMemory, glm::ivec2 &imageSize)
	{
		int texChannels;
	    int width;
	    int height;
		stbi_uc* pixels = stbi_load(path.c_str(), &width, &height, &texChannels, STBI_rgb_alpha);
	    LogAssert(pixels, "failed to load texture image!", true)


		const VkDeviceSize deviceImageSize = static_cast<VkDeviceSize>(width) * height * 4;

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;

		Core::Buffer::CreateStagingBuffer<stbi_uc>(vulkanContext, deviceImageSize, stagingBuffer, stagingBufferMemory, pixels);

		stbi_image_free(pixels);

		Image::CreateImage(vulkanContext, width, height, 1, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image, imageMemory);
		Image::TransitionAndCopyAndImageBuffer(vulkanContext, stagingBuffer, image, width, height);

		vkDestroyBuffer(vulkanContext->device, stagingBuffer, nullptr);
		vkFreeMemory(vulkanContext->device, stagingBufferMemory, nullptr);

	    imageSize = {width, height};
	}
}


namespace Image
{
	void CreateImage(const VulkanContext* vulkanContext, uint32_t width, uint32_t height, uint32_t mipLevels,
		VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
		VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
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

	void CreateImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags,
		VkImageView& imageView)
	{
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

	void CreateSampler(VulkanContext* vulkanContext, VkSampler& sampler)
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
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 0.0f;

		VulkanCheck(vkCreateSampler(vulkanContext->device, &samplerInfo, nullptr, &sampler), "Failed to create texture sampler!")
	}

	bool HasStencilComponent(VkFormat format)
	{
		return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
	}

	void TransitionAndCopyAndImageBuffer(VulkanContext* vulkanContext, VkBuffer srcBuffer, VkImage dstImage, uint32_t width, uint32_t height)
	{
		//Create Command Buffer
		CommandBuffer commandBuffer{};
		CommandBufferManager::CreateCommandBufferSingleUse(vulkanContext, commandBuffer);

		//Transition the image to transfer destination
		tools::InsertImageMemoryBarrier(
			commandBuffer.Handle, dstImage,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			{ VK_IMAGE_ASPECT_COLOR_BIT, 0,1,0,1 });

		VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;

		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;

		region.imageOffset = { 0, 0, 0 };
		region.imageExtent = { width, height, 1 };

		vkCmdCopyBufferToImage(commandBuffer.Handle, srcBuffer, dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);


		//Transition the image to shader read
		tools::InsertImageMemoryBarrier(
			commandBuffer.Handle, dstImage,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			{ VK_IMAGE_ASPECT_COLOR_BIT, 0,1,0,1 });	


		CommandBufferManager::EndCommandBufferSingleUse(vulkanContext, commandBuffer);
	}
}

Texture::Texture(const std::string &path, VulkanContext *vulkanContext)
{
    ImageLoader::CreateTexture(path, vulkanContext, m_Image, m_ImageMemory, m_ImageSize);
    Image::CreateImageView(vulkanContext->device, m_Image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT,m_ImageView);
    Image::CreateSampler(vulkanContext, m_Sampler);

    m_ImTexture = std::make_unique<ImGuiTexture>(m_Sampler, m_ImageView);
}

Texture::Texture(Texture &&other) noexcept
{
    if (&other != this) {
        m_Image = other.m_Image;
        m_ImageMemory = other.m_ImageMemory;
        m_ImageView = other.m_ImageView;
        m_Sampler = other.m_Sampler;
        m_ImTexture = std::move(other.m_ImTexture);
        m_ImageSize = other.m_ImageSize;
    }
}
void Texture::ProperBind(int bindingNumber, Descriptor::DescriptorWriter &descriptorWriter)
{
    descriptorWriter.WriteImage(bindingNumber, m_ImageView, m_Sampler, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
}

void Texture::Cleanup(VkDevice device) const
{
    m_ImTexture->Cleanup();
    vkDestroySampler(device, m_Sampler, nullptr);
    vkDestroyImageView(device, m_ImageView, nullptr);
    vkFreeMemory(device, m_ImageMemory, nullptr);
    vkDestroyImage(device, m_Image, nullptr);
}
void Texture::OnImGui() const
{
    m_ImTexture->Render(ImVec2(m_ImageSize.x / 5.0f , m_ImageSize.y  / 5.0f));
}
