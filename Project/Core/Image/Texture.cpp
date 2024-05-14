#include "Texture.h"

#include <utility>
#include "vulkanbase/VulkanTypes.h"

Texture::Texture(std::string path, ::VulkanContext *vulkanContext) :
    VulkanContext(vulkanContext), m_Path(std::move(path)) {}

Texture::Texture(Texture &&other) noexcept
{
    if (&other != this)
    {
        m_Image = other.m_Image;
        m_ImageMemory = other.m_ImageMemory;
        m_ImageView = other.m_ImageView;
        m_Sampler = other.m_Sampler;
        m_ImageSize = other.m_ImageSize;
    }
}
void Texture::ProperBind(int bindingNumber, Descriptor::DescriptorWriter &descriptorWriter) const
{
    descriptorWriter.WriteImage(bindingNumber, m_ImageView, m_Sampler, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
}



void Texture::Cleanup(VkDevice device) const
{
    vkDestroySampler(device, m_Sampler, nullptr);
    vkDestroyImageView(device, m_ImageView, nullptr);
    vkFreeMemory(device, m_ImageMemory, nullptr);
    vkDestroyImage(device, m_Image, nullptr);
}