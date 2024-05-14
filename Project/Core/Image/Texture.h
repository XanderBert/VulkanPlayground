#pragma once
#include <string>
#include <glm/vec2.hpp>

#include "Core/Descriptor.h"
#include "vulkanbase/VulkanTypes.h"

class Texture
{
public:
    Texture(std::string  path,  VulkanContext* vulkanContext);
    virtual ~Texture() = default;
    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;
    Texture(Texture &&other) noexcept;
    Texture& operator=(Texture&&) = delete;

    void ProperBind(int bindingNumber, Descriptor::DescriptorWriter& descriptorWriter) const;

    virtual void Cleanup(VkDevice device) const;

    virtual void TransitionAndCopyImageBuffer(VkBuffer srcBuffer) = 0;
    virtual void InitTexture() = 0;
protected:

    VulkanContext* VulkanContext{};

    std::string m_Path{};

    glm::ivec2 m_ImageSize{};
    uint32_t m_MipLevels{};

    VkImage m_Image{};
    VkDeviceMemory m_ImageMemory{};
    VkImageView m_ImageView{};
    VkSampler m_Sampler{};
};


// Texture concept
template<typename TextureType>
concept TextureConcept =    std::is_base_of_v<Texture, TextureType> &&
                            std::is_constructible_v<TextureType, const std::string&, VulkanContext*> &&
                            std::is_move_constructible_v<TextureType>;