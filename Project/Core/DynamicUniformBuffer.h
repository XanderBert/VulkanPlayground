#pragma once
#include <vector>
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>
#include <vulkan/vulkan.h>
#include "vulkanbase/VulkanTypes.h"

namespace Descriptor
{
	class DescriptorWriter;
	class DescriptorBuilder;
}


//TODO: pad the dynamic buffer to 256 bytes
//TODO: return actual pointers to the data instead of the handle, Or make a handle struct
class DynamicBuffer final
{
public:
	DynamicBuffer() = default;
	~DynamicBuffer() = default;

	DynamicBuffer& operator=(const DynamicBuffer&) = delete;
	DynamicBuffer(const DynamicBuffer&) = delete;
	DynamicBuffer(DynamicBuffer&&) = delete;
	DynamicBuffer& operator=(DynamicBuffer&&) = delete;

	void Init(VulkanContext* vulkanContext);
	void ProperBind(int bindingNumber, const VkDescriptorSet& descriptorSet, Descriptor::DescriptorWriter& descriptorWriter, VulkanContext* vulkanContext) const;


	void Cleanup(VkDevice device) const;

	uint16_t AddVariable(const glm::vec4& value);
	uint16_t AddVariable(const glm::mat4& matrix);
	void UpdateVariable(uint16_t handle, const glm::mat4& matrix);
	void UpdateVariable(uint16_t handle, const glm::vec4& value);

private:
	const float* GetData() const;
	size_t GetSize() const;

	uint16_t Insert(const float* dataPtr, uint8_t size);
	void Update(uint16_t handle, const float* dataPtr, uint8_t size);
	std::vector<float> m_Data;


	VkBuffer m_UniformBuffer;
	VkDeviceMemory m_UniformBuffersMemory;
	void* m_UniformBuffersMapped;
};