#include "ImageLoader.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#include <ktx/include/ktx.h>
#include <ImGuiFileDialog.h>

#include "Texture.h"
#include "Core/CommandBuffer.h"
#include "Core/Logger.h"
#include "vulkanbase/VulkanBase.h"

namespace Image
{
	void CreateImage(uint32_t width, uint32_t height, uint32_t mipLevels,
		VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
		VkImage& image, VmaAllocation& imageMemory, TextureType textureType)
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

        if(textureType == TextureType::TEXTURE_CUBE)
        {
            imageInfo.arrayLayers = 6;
            imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
        }

	    VmaAllocationCreateInfo props{};
	    props.usage = VMA_MEMORY_USAGE_AUTO ;
	    props.priority = 1.0f;

	    //if the image size is big (greater then 1440p ) then we should use the dedicated memory
	    //Use for:
	    // ● Render targets, depth-stencil, UAV
        // ● Very large buffers and images (dozens of MiB)
        // ● Large allocations that may need to be resized (freed and reallocated) at run-time
        if(constexpr int treshold = 2560 * 1440; width * height > treshold) props.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;

        //Create the image
	    VulkanCheck(vmaCreateImage(Allocator::VmaAllocator, &imageInfo, &props, &image, &imageMemory, nullptr), "Failed To Create Image");
	}

	void CreateImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, VkImageView &imageView, TextureType textureType)
    {
	    //TODO: Should mips be in .levelCount?
	    const uint32_t layerCount = textureType == TextureType::TEXTURE_CUBE ? 6 : 1;
	    const VkImageViewCreateInfo viewInfo
        {
          VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
          nullptr,
          0,
          image,
          static_cast<VkImageViewType>(textureType),
          format,
          {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY},
          {aspectFlags, 0, 1, 0, layerCount}
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





std::pair<VkBuffer, VmaAllocation> stbi::CreateImage(const std::filesystem::path &path, glm::ivec2 &imageSize, uint32_t &mipLevels)
{
    LogInfo(path.generic_string());

    int channels{};
    stbi_uc *pixels = stbi_load(path.generic_string().c_str(), &imageSize.x, &imageSize.y, &channels, STBI_rgb_alpha);

    LogAssert(pixels, "failed to load texture image!", true)
    mipLevels = 1;


    const VkDeviceSize deviceImageSize = static_cast<VkDeviceSize>(imageSize.x) * imageSize.y * 4;
    VkBuffer stagingBuffer;
    VmaAllocation stagingBufferMemory;
    Core::Buffer::CreateStagingBuffer<stbi_uc>(deviceImageSize, stagingBuffer, stagingBufferMemory, pixels);

    stbi_image_free(pixels);


    return {stagingBuffer, stagingBufferMemory};
}
std::pair<VkBuffer, VmaAllocation> stbi::CreateImageFromMemory(const std::uint8_t* data, size_t size, glm::ivec2 &imageSize, uint32_t &mipLevels)
{
    int channels{};
    stbi_uc *pixels = stbi_load_from_memory(data, size, &imageSize.x, &imageSize.y, &channels, 4);
    LogAssert(pixels, "failed to load texture image!", true)
    mipLevels = 1;

    const VkDeviceSize deviceImageSize = static_cast<VkDeviceSize>(imageSize.x) * imageSize.y * 4;
    VkBuffer stagingBuffer;
    VmaAllocation stagingBufferMemory;
    Core::Buffer::CreateStagingBuffer<stbi_uc>(deviceImageSize, stagingBuffer, stagingBufferMemory,pixels);

    stbi_image_free(pixels);

    return {stagingBuffer, stagingBufferMemory};
}
std::pair<VkBuffer, VmaAllocation> ktx::CreateImage(const std::filesystem::path &path, glm::ivec2 &imageSize, uint32_t &mipLevels, ktxTexture **texture)
{
    LogAssert(path.extension() == ".ktx", path.generic_string() + " is not a .ktx file", true)

    // TODO: KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT -  should not be set, It should be directly loaded in the staging buffer
    auto errorCode = ktxTexture_CreateFromNamedFile(path.generic_string().c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, texture);



    LogAssert(errorCode == KTX_SUCCESS, "Failed to load texture image!", true)
    LogAssert((*texture) != nullptr, "The KTX Texture is not valid", true)

    // Get properties required for using and upload texture data from the ktx texture object
    imageSize = {(*texture)->baseWidth, (*texture)->baseHeight};
    mipLevels = (*texture)->numLevels;

    ktx_uint8_t *ktxTextureData = ktxTexture_GetData((*texture));
    ktx_size_t ktxTextureSize = ktxTexture_GetSize((*texture));

    VkBuffer stagingBuffer;
    VmaAllocation stagingBufferMemory;

    Core::Buffer::CreateStagingBuffer<ktx_uint8_t>(ktxTextureSize, stagingBuffer, stagingBufferMemory, ktxTextureData);

    return {stagingBuffer, stagingBufferMemory};
}
std::pair<VkBuffer, VmaAllocation> ktx::CreateImageFromMemory(const std::uint8_t* data, size_t size, glm::ivec2 &imageSize, uint32_t &mipLevels, ktxTexture **texture)
{
    // TODO: KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT -  should not be set, It should be directly loaded in the staging buffer
    auto errorCode = ktxTexture_CreateFromMemory(data, size, KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, texture);
    LogAssert(errorCode == KTX_SUCCESS, "Failed to load texture image!", true)
    LogAssert((*texture) != nullptr, "The KTX Texture is not valid", true)

    // Get properties required for using and upload texture data from the ktx texture object
    imageSize = {(*texture)->baseWidth, (*texture)->baseHeight};
    mipLevels = (*texture)->numLevels;

    ktx_uint8_t *ktxTextureData = ktxTexture_GetData((*texture));
    ktx_size_t ktxTextureSize = ktxTexture_GetSize((*texture));

    VkBuffer stagingBuffer;
    VmaAllocation stagingBufferMemory;

    Core::Buffer::CreateStagingBuffer<ktx_uint8_t>(ktxTextureSize, stagingBuffer, stagingBufferMemory, ktxTextureData);

    return {stagingBuffer, stagingBufferMemory};
}
