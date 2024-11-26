#pragma once
#include <vector>
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>
#include <vulkan/vulkan.h>

#include "DescriptorSet.h"
#include "vulkanbase/VulkanTypes.h"
#include "Core/VmaUsage.h"

enum class DescriptorType;

namespace Descriptor
{
	class DescriptorWriter;
	class DescriptorBuilder;
}

enum class BufferType
{
    UniformBuffer = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
    StorageBuffer = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
};

//TODO: pad the dynamic buffer to 256 bytes
//TODO: return actual pointers to the data instead of the handle, Or make a handle struct
class DynamicBuffer final
{
public:
	explicit DynamicBuffer() = default;
	~DynamicBuffer() = default;

	DynamicBuffer& operator=(const DynamicBuffer&) = delete;
	DynamicBuffer(const DynamicBuffer&) = delete;
    DynamicBuffer(DynamicBuffer&& other) noexcept;
    DynamicBuffer& operator=(DynamicBuffer&& other) noexcept = delete;

	void Init(BufferType bufferType);
	void Bind(Descriptor::DescriptorWriter& descriptorWriter, DescriptorType descriptorType) const;
    void Cleanup(VkDevice device) const;

	uint16_t AddVariable(float value);
	uint16_t AddVariable(const glm::vec2& value);
	uint16_t AddVariable(const glm::vec4& value);
	uint16_t AddVariable(const glm::mat4& matrix);

	void UpdateVariable(uint16_t handle, float value);
	void UpdateVariable(uint16_t handle, const glm::vec2& value);
	void UpdateVariable(uint16_t handle, const glm::mat4& matrix);
	void UpdateVariable(uint16_t handle, const glm::vec4& value);

    void OnImGui();


private:
	[[nodiscard]] const float* GetData() const;
	[[nodiscard]] size_t GetSize() const;

	uint16_t Insert(const float* dataPtr, uint8_t size);
	void Update(uint16_t handle, const float* dataPtr, uint8_t size);
	std::vector<float> m_Data;

	VkBuffer m_UniformBuffer{};
	VmaAllocation m_UniformBuffersMemory{};
	void* m_UniformBuffersMapped{};
};