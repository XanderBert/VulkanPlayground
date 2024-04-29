#pragma once
#include <string>
#include <vulkan/vulkan_core.h>
#include <glm/vec2.hpp>

#include "Core/Descriptor.h"
#include "Core/ImGuiWrapper.h"
#include "vulkanbase/VulkanTypes.h"


namespace Image
{
	void CreateImage(
		const VulkanContext* vulkanContext, 
		uint32_t width, 
		uint32_t height, 
		uint32_t mipLevels, 
		VkSampleCountFlagBits numSamples, 
		VkFormat format, 
		VkImageTiling tiling, 
		VkImageUsageFlags usage, 
		VkMemoryPropertyFlags properties, 
		VkImage& image, 
		VkDeviceMemory& imageMemory);

	void CreateImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, VkImageView& imageView);
	void CreateSampler(VulkanContext* vulkanContext, VkSampler& sampler);	
	bool HasStencilComponent(VkFormat format);
	void TransitionAndCopyAndImageBuffer(VulkanContext* vulkanContext, VkBuffer srcBuffer, VkImage dstImage, uint32_t width, uint32_t height);
}

namespace ImageLoader
{
	void CreateTexture(const std::string& path, VulkanContext* vulkanContext, VkImage& image, VkDeviceMemory& imageMemory, glm::ivec2 &imageSize);

}

class Texture
{
public:
	Texture(const std::string& path,  VulkanContext* vulkanContext);
	~Texture() = default;
	Texture(const Texture&) = delete;
	Texture& operator=(const Texture&) = delete;
	Texture(Texture &&other) noexcept;
    ;
	Texture& operator=(Texture&&) = delete;

    void ProperBind(int bindingNumber, Descriptor::DescriptorWriter& descriptorWriter);
    void Cleanup(VkDevice device) const;

    void OnImGui() const;
private:
    std::unique_ptr<ImGuiTexture> m_ImTexture;

    glm::ivec2 m_ImageSize{};

	VkImage m_Image{};
	VkDeviceMemory m_ImageMemory{};
	VkImageView m_ImageView{};
	VkSampler m_Sampler{};
};