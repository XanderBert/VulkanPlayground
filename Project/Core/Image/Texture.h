#pragma once
#include <variant>
#include <filesystem>
#include <glm/vec2.hpp>
#include <ktx.h>

#include "Core/Descriptor.h"
#include "ImageLoader.h"
#include "Core/ImGuiWrapper.h"
#include "vulkanbase/VulkanTypes.h"

enum class ColorType : uint8_t
{
    SRGB = VK_FORMAT_R8G8B8A8_SRGB,
    LINEAR = VK_FORMAT_R8G8B8A8_UNORM,
};

enum class TextureType : uint8_t
{
    TEXTURE_2D = VK_IMAGE_VIEW_TYPE_2D,
    TEXTURE_CUBE = VK_IMAGE_VIEW_TYPE_CUBE
};


enum class DescriptorImageType : uint8_t
{
    SAMPLED_IMAGE = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
    STORAGE_IMAGE = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
};


struct  ImageInMemory
{
    VkBuffer staginBuffer;
    VmaAllocation stagingBufferMemory;
    glm::ivec2 imageSize;
    uint32_t mipLevels;
};

using TextureData = std::variant<ktxVulkanTexture, ImageInMemory>;
using ImageMemory = std::variant<VmaAllocation, VkDeviceMemory>;

//TODO only take a ImageInMemory To Construct a actual image
class Texture final
{
public:
    //Used for a texture that is loaded from a file or from memory
    Texture(const std::variant<std::filesystem::path,ImageInMemory>& pathOrImage, VulkanContext *vulkanContext, ColorType colorType, TextureType textureType);

    //used to create a empty texture that can be used as a output texture
    Texture(VulkanContext *vulkanContext,const glm::ivec2& extent, ColorType colorType, TextureType textureType);


    ~Texture() = default;
    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;
    Texture(Texture &&other) noexcept = delete;
    Texture& operator=(Texture&&) = delete;

    void SetDescriptorImageType(DescriptorImageType descriptorImageType);

    void OnImGui();

    void ProperBind(int bindingNumber, Descriptor::DescriptorWriter& descriptorWriter) const;

    void Cleanup(VkDevice device) const;


private:
    void InitTexture(const TextureData& loadedImage);
    void InitTexture(const std::filesystem::path& path);
    void InitEmptyTexture();

    void TransitionAndCopyImageBuffer(VkBuffer srcBuffer) const;
    void TransitionToWritableImageLayout() const;

    static void CleanupImage(VkDeviceMemory deviceMemory, VkImage image);
    static void CleanupImage(VmaAllocation deviceMemory, VkImage image);
private:
    VulkanContext* m_pContext{};
    std::optional<std::filesystem::path> m_Path{};

    glm::ivec2 m_ImageSize{};
    uint32_t m_MipLevels{};

    ImageMemory m_ImageMemory{};

    VkImage m_Image{};
    VkImageView m_ImageView{};
    VkSampler m_Sampler{};


    ColorType m_ColorType{};
    TextureType m_TextureType{};
    DescriptorImageType m_DescriptorImageType{DescriptorImageType::SAMPLED_IMAGE};

    std::unique_ptr<ImGuiTexture> m_ImGuiTexture{};
};