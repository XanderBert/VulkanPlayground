#include "Texture.h"

#include <filesystem>
#include <utility>
#include "vulkanbase/VulkanTypes.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#include <ktx/include/ktx.h>





Texture::Texture(const std::filesystem::path &path, ::VulkanContext *vulkanContext, ColorType colorType) :
    VulkanContext(vulkanContext), m_ColorType(colorType)
{
    m_Path = VulkanContext::GetAssetPath(path.generic_string());
}

Texture::Texture(const LoadedImage &loadedImage, ::VulkanContext *vulkanContext, ColorType colorType)
    : VulkanContext(vulkanContext), m_ImageSize(loadedImage.imageSize), m_MipLevels(loadedImage.mipLevels), m_ColorType(colorType)
{}

Texture::Texture(Texture &&other) noexcept
{
    if (&other != this)
    {
        m_Image = other.m_Image;
        m_ImageMemory = other.m_ImageMemory;
        m_ImageView = other.m_ImageView;
        m_Sampler = other.m_Sampler;
        m_ImageSize = other.m_ImageSize;
        m_MipLevels = other.m_MipLevels;
        m_Path = std::move(other.m_Path);
        m_ColorType = other.m_ColorType;
    }
}
void Texture::ProperBind(int bindingNumber, Descriptor::DescriptorWriter &descriptorWriter) const
{
    descriptorWriter.WriteImage(bindingNumber, m_ImageView, m_Sampler, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
}



void Texture::Cleanup(VkDevice device) const {
    vkDestroySampler(device, m_Sampler, nullptr);
    vkDestroyImageView(device, m_ImageView, nullptr);
    vkFreeMemory(device, m_ImageMemory, nullptr);
    vkDestroyImage(device, m_Image, nullptr);
}


std::pair<VkBuffer, VkDeviceMemory> stbi::CreateImage(VulkanContext *vulkanContext, const std::filesystem::path &path,
                                                      glm::ivec2 &imageSize, uint32_t &mipLevels) {
    LogInfo(path.generic_string());

    int channels{};
    stbi_uc *pixels = stbi_load(path.generic_string().c_str(), &imageSize.x, &imageSize.y, &channels, STBI_rgb_alpha);

    LogAssert(pixels, "failed to load texture image!", true)
    mipLevels = 1;


    const VkDeviceSize deviceImageSize = static_cast<VkDeviceSize>(imageSize.x) * imageSize.y * 4;
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    Core::Buffer::CreateStagingBuffer<stbi_uc>(vulkanContext, deviceImageSize, stagingBuffer, stagingBufferMemory,
                                               pixels);

    stbi_image_free(pixels);


    return {stagingBuffer, stagingBufferMemory};
}
std::pair<VkBuffer, VkDeviceMemory> stbi::CreateImageFromMemory(VulkanContext *vulkanContext,const std::uint8_t* data, size_t size, glm::ivec2 &imageSize, uint32_t &mipLevels)
{
    int channels{};
    stbi_uc *pixels = stbi_load_from_memory(data, size, &imageSize.x, &imageSize.y, &channels, 4);
    LogAssert(pixels, "failed to load texture image!", true)
    mipLevels = 1;

    const VkDeviceSize deviceImageSize = static_cast<VkDeviceSize>(imageSize.x) * imageSize.y * 4;
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    Core::Buffer::CreateStagingBuffer<stbi_uc>(vulkanContext, deviceImageSize, stagingBuffer, stagingBufferMemory,pixels);

    stbi_image_free(pixels);

    return {stagingBuffer, stagingBufferMemory};
}

std::pair<VkBuffer, VkDeviceMemory> ktx::CreateImage(VulkanContext *vulkanContext, const std::filesystem::path &path, glm::ivec2 &imageSize, uint32_t &mipLevels, ktxTexture **texture)
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
    VkDeviceMemory stagingBufferMemory;

    Core::Buffer::CreateStagingBuffer<ktx_uint8_t>(vulkanContext, ktxTextureSize, stagingBuffer, stagingBufferMemory, ktxTextureData);

    return {stagingBuffer, stagingBufferMemory};
}
std::pair<VkBuffer, VkDeviceMemory> ktx::CreateImageFromMemory(VulkanContext *vulkanContext, const std::uint8_t* data, size_t size, glm::ivec2 &imageSize, uint32_t &mipLevels, ktxTexture **texture)
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
    VkDeviceMemory stagingBufferMemory;

    Core::Buffer::CreateStagingBuffer<ktx_uint8_t>(vulkanContext, ktxTextureSize, stagingBuffer, stagingBufferMemory, ktxTextureData);

    return {stagingBuffer, stagingBufferMemory};
}
