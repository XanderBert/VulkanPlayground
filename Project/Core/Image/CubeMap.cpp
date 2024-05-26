#include "CubeMap.h"


#include "Core/CommandBuffer.h"
#include "ImageLoader.h"

CubeMap::CubeMap(const std::filesystem::path &path, ::VulkanContext *vulkanContext, ColorType colorType) :
    Texture(path, vulkanContext, colorType) {
    InitTexture(std::nullopt);
}
CubeMap::CubeMap(const LoadedImage &loadedImage, ::VulkanContext *vulkanContext, ColorType colorType)
    :Texture(loadedImage, vulkanContext, colorType)
{
    InitTexture(loadedImage);
}

void CubeMap::TransitionAndCopyImageBuffer(VkBuffer srcBuffer, ktxTexture* texture)
{
    LogAssert(texture != nullptr, "The KTX Texture was allready destroyed", true)

    //Create Command Buffer
	CommandBuffer commandBuffer{};
	CommandBufferManager::CreateCommandBufferSingleUse(VulkanContext, commandBuffer);

    std::vector<VkBufferImageCopy> bufferCopyRegions;

    for (uint32_t face{}; face < 6; ++face)
    {
        for(uint32_t mipLevel{}; mipLevel < m_MipLevels; ++mipLevel)
        {
            ktx_size_t offset;
            KTX_error_code ret = ktxTexture_GetImageOffset(texture, mipLevel, 0, face, &offset);
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

void CubeMap::InitTexture(std::optional<LoadedImage> loadedImage)
{
    std::pair<VkBuffer, VkDeviceMemory> stagingSources;
    ktxTexture* texture{};

    if(!loadedImage.has_value())
    {
        stagingSources = ktx::CreateImage(VulkanContext, m_Path, m_ImageSize, m_MipLevels, &texture);
    }else
    {
        stagingSources.first = loadedImage->staginBuffer;
        stagingSources.second = loadedImage->stagingBufferMemory;
        texture = loadedImage->texture.value();
    }


    Image::CreateImage(VulkanContext, m_ImageSize.x, m_ImageSize.y, m_MipLevels, VK_SAMPLE_COUNT_1_BIT, static_cast<VkFormat>(m_ColorType), VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_Image, m_ImageMemory, true);

    TransitionAndCopyImageBuffer(stagingSources.first, texture);
    ktxTexture_Destroy(texture);


    vkDestroyBuffer(VulkanContext->device, stagingSources.first, nullptr);
    vkFreeMemory(VulkanContext->device, stagingSources.second, nullptr);

    Image::CreateCubeImageView(VulkanContext->device, m_Image, static_cast<VkFormat>(m_ColorType), VK_IMAGE_ASPECT_COLOR_BIT, m_ImageView);
    Image::CreateSampler(VulkanContext, m_Sampler, m_MipLevels);
}
