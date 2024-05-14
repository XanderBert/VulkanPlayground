#include "CubeMap.h"
#include <ktx/include/ktx.h>

#include "Core/CommandBuffer.h"
#include "ImageLoader.h"

CubeMap::CubeMap(const std::string &path, ::VulkanContext *vulkanContext)
    : Texture(path, vulkanContext)
{
    //Check if its a .ktx file
    const std::string extension = path.substr(path.find_last_of('.') + 1);
    LogAssert(extension == "ktx", "cannot make a cubemap of a file that is not a .ktx file", true)

    InitTexture();
}

void CubeMap::TransitionAndCopyImageBuffer(VkBuffer srcBuffer)
{
    //Create Command Buffer
	CommandBuffer commandBuffer{};
	CommandBufferManager::CreateCommandBufferSingleUse(VulkanContext, commandBuffer);

    std::vector<VkBufferImageCopy> bufferCopyRegions;

    LogAssert(m_LoadingTexture != nullptr, "The KTX Texture was allready destroyed", true)

    for (uint32_t face{}; face < 6; ++face)
    {
        for(uint32_t mipLevel{}; mipLevel < m_MipLevels; ++mipLevel)
        {
            ktx_size_t offset;
            KTX_error_code ret = ktxTexture_GetImageOffset(m_LoadingTexture, mipLevel, 0, face, &offset);
            LogAssert(ret == KTX_SUCCESS, "Failed to get image offset",true );

            VkBufferImageCopy region{};
            region.bufferOffset = offset;

            region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            region.imageSubresource.mipLevel = mipLevel;
            region.imageSubresource.baseArrayLayer = face;
            region.imageSubresource.layerCount = 1;

            region.imageExtent.width = m_ImageSize.x >> mipLevel;
            region.imageExtent.height = m_ImageSize.y >> mipLevel;
            region.imageExtent.depth = 1;

            bufferCopyRegions.push_back(region);
        }
    }



    VkImageSubresourceRange subresourceRange = {};
    subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subresourceRange.baseMipLevel = 0;
    subresourceRange.levelCount = m_MipLevels;
    subresourceRange.layerCount = static_cast<uint32_t>(bufferCopyRegions.size() / m_MipLevels);

    //Transition the image to transfer destination
    tools::InsertImageMemoryBarrier(
        commandBuffer.Handle, m_Image,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        subresourceRange);

    vkCmdCopyBufferToImage(commandBuffer.Handle, srcBuffer, m_Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, static_cast<uint32_t>(bufferCopyRegions.size()), bufferCopyRegions.data());

    //Transition the image to shader read
    tools::InsertImageMemoryBarrier(
        commandBuffer.Handle, m_Image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        subresourceRange);


    CommandBufferManager::EndCommandBufferSingleUse(VulkanContext, commandBuffer);
}

void CubeMap::InitTexture()
{
    auto errorCode = ktxTexture_CreateFromNamedFile(m_Path.c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &m_LoadingTexture);
    LogAssert(errorCode == KTX_SUCCESS, "Failed to load texture image!", true)

    // Get properties required for using and upload texture data from the ktx texture object
    m_ImageSize = {m_LoadingTexture->baseWidth, m_LoadingTexture->baseHeight};
    m_MipLevels = m_LoadingTexture->numLevels;

    ktx_uint8_t *ktxTextureData = ktxTexture_GetData(m_LoadingTexture);
    ktx_size_t ktxTextureSize = ktxTexture_GetSize(m_LoadingTexture);

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    Core::Buffer::CreateStagingBuffer<ktx_uint8_t>(VulkanContext, ktxTextureSize, stagingBuffer, stagingBufferMemory, ktxTextureData);

    Image::CreateImage(VulkanContext, m_ImageSize.x, m_ImageSize.y, m_MipLevels, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_Image, m_ImageMemory, true);
    TransitionAndCopyImageBuffer(stagingBuffer);

    vkDestroyBuffer(VulkanContext->device, stagingBuffer, nullptr);
    vkFreeMemory(VulkanContext->device, stagingBufferMemory, nullptr);
    ktxTexture_Destroy(m_LoadingTexture);


    Image::CreateCubeImageView(VulkanContext->device, m_Image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, m_ImageView);
    Image::CreateSampler(VulkanContext, m_Sampler, m_MipLevels);
}
