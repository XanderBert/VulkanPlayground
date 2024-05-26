#pragma once
#include <array>
#include <filesystem>
#include <glm/vec2.hpp>
#include <ktx.h>
#include <string>

#include "Core/Descriptor.h"
#include "ImageLoader.h"
#include "vulkanbase/VulkanTypes.h"

enum class ColorType
{
    SRGB = VK_FORMAT_R8G8B8A8_SRGB,
    LINEAR = VK_FORMAT_R8G8B8A8_UNORM
};

struct  LoadedImage
{
    VkBuffer staginBuffer;
    VkDeviceMemory stagingBufferMemory;
    glm::ivec2 imageSize;
    uint32_t mipLevels;

    std::optional<ktxTexture*> texture;
};


class Texture
{
public:
    Texture(const std::filesystem::path &path,  VulkanContext* vulkanContext, ColorType colorType);
    Texture(const LoadedImage &loadedImage, VulkanContext* vulkanContext, ColorType colorType);

    virtual ~Texture() = default;
    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;
    Texture(Texture &&other) noexcept;
    Texture& operator=(Texture&&) = delete;

    void ProperBind(int bindingNumber, Descriptor::DescriptorWriter& descriptorWriter) const;

    virtual void Cleanup(VkDevice device) const;

    virtual void InitTexture(std::optional<LoadedImage> loadedImage = std::nullopt) = 0;
protected:

    VulkanContext* VulkanContext{};
    std::filesystem::path m_Path{};

    glm::ivec2 m_ImageSize{};
    uint32_t m_MipLevels{};
    VkDeviceMemory m_ImageMemory{};

    VkImage m_Image{};
    VkImageView m_ImageView{};
    VkSampler m_Sampler{};


    ColorType m_ColorType{};
};


// Texture concept

//Texture can also be constructed as  Texture(const LoadedImage &loadedImage, VulkanContext* vulkanContext, ColorType colorType);
template<typename TextureType>
concept TextureConcept =    std::is_base_of_v<Texture, TextureType> &&
                            std::is_constructible_v<TextureType, const std::filesystem::path&, VulkanContext*, ColorType> ||
                            std::is_constructible_v<TextureType, const LoadedImage&, VulkanContext*, ColorType> &&
                            std::is_move_constructible_v<TextureType>;









//stbi loader
namespace stbi
{
    std::pair<VkBuffer, VkDeviceMemory> CreateImage(VulkanContext* vulkanContext, const std::filesystem::path &path, glm::ivec2 &imageSize, uint32_t &mipLevels);
    std::pair<VkBuffer, VkDeviceMemory> CreateImageFromMemory(VulkanContext *vulkanContext,const  std::vector<std::uint8_t>& data, glm::ivec2 &imageSize, uint32_t &mipLevels);
}

//ktx Loader
namespace ktx
{
    std::pair<VkBuffer, VkDeviceMemory> CreateImage(VulkanContext *vulkanContext, const std::filesystem::path &path, glm::ivec2 &imageSize, uint32_t &mipLevels, ktxTexture** texture);
    std::pair<VkBuffer, VkDeviceMemory> CreateImageFromMemory(VulkanContext *vulkanContext,const  std::vector<std::uint8_t>& data, glm::ivec2 &imageSize, uint32_t &mipLevels, ktxTexture** texture);
}

