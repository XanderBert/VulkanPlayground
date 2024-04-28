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
	explicit DynamicBuffer() = default;
	~DynamicBuffer() = default;

	DynamicBuffer& operator=(const DynamicBuffer&) = delete;
	DynamicBuffer(const DynamicBuffer&) = delete;

    DynamicBuffer(DynamicBuffer&& other) noexcept
    {
	    if(this != &other)
        {
            m_Data = std::move(other.m_Data);
            m_UniformBuffer = other.m_UniformBuffer;
            m_UniformBuffersMemory = other.m_UniformBuffersMemory;
            m_UniformBuffersMapped = other.m_UniformBuffersMapped;

            other.m_UniformBuffer = nullptr;
            other.m_UniformBuffersMemory = nullptr;
            other.m_UniformBuffersMapped = nullptr;
        }
	}
	DynamicBuffer& operator=(DynamicBuffer&& other) noexcept = delete;

	void Init(VulkanContext* vulkanContext);
	void ProperBind(int bindingNumber, const VkDescriptorSet& descriptorSet, Descriptor::DescriptorWriter& descriptorWriter, VulkanContext* vulkanContext) const;
    void FullRebind(int bindingNumber, const VkDescriptorSet& descriptorSet, Descriptor::DescriptorWriter& descriptorWriter, VulkanContext* vulkanContext) const;
    void Cleanup(VkDevice device) const;

	uint16_t AddVariable(const glm::vec4& value);
	uint16_t AddVariable(const glm::mat4& matrix);
	void UpdateVariable(uint16_t handle, const glm::mat4& matrix);
	void UpdateVariable(uint16_t handle, const glm::vec4& value);

    void OnImGui() const;
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