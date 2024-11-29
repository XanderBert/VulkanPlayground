#pragma once
#include <filesystem>
#include <variant>
#include <glm/vec2.hpp>

#include "ImageLoader.h"
#include "Core/Descriptor.h"
#include "Core/DescriptorSet.h"
#include "Core/ImGuiWrapper.h"
#include "vulkanbase/VulkanTypes.h"

enum class DescriptorType;

enum class ColorType : uint8_t
{
	SRGB = VK_FORMAT_R8G8B8A8_SRGB,
	LINEAR = VK_FORMAT_R8G8B8A8_UNORM,
	R16U = VK_FORMAT_R16_UNORM,
};

enum class TextureType : uint8_t
{
	TEXTURE_2D = VK_IMAGE_VIEW_TYPE_2D,
	TEXTURE_CUBE = VK_IMAGE_VIEW_TYPE_CUBE
};




struct ImageInMemory
{
	VkBuffer stagingBuffer;
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
	Texture(const std::variant<std::filesystem::path, ImageInMemory> &pathOrImage, VulkanContext *vulkanContext, ColorType colorType, TextureType textureType);

	//used to create an empty texture that can be used as an output texture
	Texture(VulkanContext *vulkanContext, const glm::ivec2 &extent, ColorType colorType, TextureType textureType);


	~Texture() = default;
	Texture(const Texture &) = delete;
	Texture &operator=(const Texture &) = delete;
	Texture(Texture &&other) noexcept = delete;
	Texture &operator=(Texture &&) = delete;

	void OnImGui();
	void Write(Descriptor::DescriptorWriter &writer, DescriptorResourceHandle newIndex, DescriptorType descriptorType);
	void Cleanup(VkDevice device);

	void TransitionToGeneralImageLayout(VkCommandBuffer commandBuffer);
	void TransitionToReadableImageLayout(VkCommandBuffer commandBuffer);
	void ClearImage(VkCommandBuffer commandBuffer) const;

	void SetOutputTexture(bool isOutputTexture);
	[[nodiscard]] bool IsOutputTexture() const;
	[[nodiscard]] DescriptorType GetDescriptorType() const;

	[[nodiscard]] bool IsPendingKill() const;

private:
	void InitTexture(const TextureData &loadedImage);
	void InitTexture(const std::filesystem::path &path);
	void InitEmptyTexture();

	void TransitionAndCopyImageBuffer(VkBuffer srcBuffer);

	void CleanupImage(VkDeviceMemory deviceMemory, VkImage image);
	void CleanupImage(VmaAllocation deviceMemory, VkImage image);

	VulkanContext *m_pContext{};
	std::optional<std::filesystem::path> m_Path{};

	glm::ivec2 m_ImageSize{};
	uint32_t m_MipLevels{};

	ImageMemory m_ImageMemory{};

	VkImage m_Image{};
	VkImageView m_ImageView{};
	VkSampler m_Sampler{};


	ColorType m_ColorType{};
	TextureType m_TextureType{};
	DescriptorType m_DescriptorImageType{DescriptorType::SampledImage};

	VkImageLayout m_BindImageLayout{VK_IMAGE_LAYOUT_UNDEFINED};

	std::unique_ptr<ImGuiTexture> m_ImGuiTexture{};

	bool m_IsOutputTexture{false};
	bool m_IsPendingKill{false};
};