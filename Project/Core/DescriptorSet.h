#pragma once
#include <memory>
#include <variant>
#include <vulkan/vulkan_core.h>

#include "Descriptor.h"
#include "DynamicUniformBuffer.h"


class ColorAttachment;
class Texture;
class DynamicBuffer;
struct GlobalDescriptor;
enum class PipelineType;
class DepthResource;
class VulkanContext;

enum class DescriptorType
{
	UniformBuffer = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
	StorageBuffer = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
	CombinedImageSampler = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
	SampledImage = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
	StorageImage = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
	UniformTexelBuffer = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,
	StorageTexelBuffer = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,
	UniformBufferDynamic = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
	StorageBufferDynamic = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC,
	InputAttachment = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
	AccelerationStructureKHR = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR,
	InlineUniformBlock = VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK
};

struct DescriptorResource
{
	// Constructor for DynamicBuffer (move semantics)
	DescriptorResource(DynamicBuffer&& buffer, DescriptorType descriptorType)
		: resource(std::in_place_type<DynamicBuffer>, std::move(buffer))
		, type(descriptorType) {}

	// Constructor for Texture
	DescriptorResource(std::shared_ptr<Texture> texture, DescriptorType descriptorType)
		: resource(std::in_place_type<std::shared_ptr<Texture>>, std::move(texture))
		, type(descriptorType) {}

	// Constructor for ColorAttachment
	DescriptorResource(ColorAttachment* attachment, DescriptorType descriptorType)
		: resource(std::in_place_type<ColorAttachment*>, attachment)
		, type(descriptorType) {}


	std::variant<DynamicBuffer, std::shared_ptr<Texture>, ColorAttachment*> resource;
	DescriptorType type;
};


class DescriptorSet
{
public:
	DescriptorSet() = default;
	~DescriptorSet() = default;

	DescriptorSet(const DescriptorSet&) = delete;
	DescriptorSet& operator=(const DescriptorSet&) = delete;
	DescriptorSet(DescriptorSet&&) = delete;
	DescriptorSet& operator=(DescriptorSet&&) = delete;

	void Initialize(const VulkanContext* pContext);

	void Cleanup(const VulkanContext* pContext);

	void Bind(VulkanContext* pContext, const VkCommandBuffer& commandBuffer, PipelineType pipelineType);

	//This returns the index for the array in the shader
	int AddResource(const DescriptorResource& resource);

	[[nodiscard]] const DescriptorResource& GetResource(int index) const;

	[[nodiscard]] const VkDescriptorSetLayout& GetDescriptorSetLayout() const;

private:
	std::vector<DescriptorResource> m_Resources;

	VkDescriptorSetLayout m_DescriptorSetLayout{};
	VkDescriptorSet m_DescriptorSet{};

	Descriptor::DescriptorWriter m_DescriptorWriter{};
	Descriptor::DescriptorBuilder m_DescriptorBuilder{};
};
