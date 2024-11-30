#pragma once
#include <cstring>
#include <vector>
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>

#include "Core/VmaUsage.h"

enum class DescriptorResourceHandle : uint32_t;
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
	DynamicBuffer() = default;
	~DynamicBuffer() = default;

	DynamicBuffer& operator=(const DynamicBuffer&) = delete;
	DynamicBuffer(const DynamicBuffer&) = delete;
    DynamicBuffer(DynamicBuffer&& other) noexcept;
    DynamicBuffer& operator=(DynamicBuffer&& other) noexcept = delete;

	void Write(Descriptor::DescriptorWriter &writer, DescriptorResourceHandle newIndex, DescriptorType descriptorType);
	void Bind() const;
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


//---------------------------------------------------------------
//-------------------------BindlessParameters-------------------------
//---------------------------------------------------------------
class BindlessParameters
{
	struct Range
	{
		uint32_t offset;
		uint32_t size;
		void *data;
	};

public:
	BindlessParameters() = default;
	~BindlessParameters() = default;

	BindlessParameters(const BindlessParameters&) = delete;
	BindlessParameters& operator=(const BindlessParameters&) = delete;
	BindlessParameters(BindlessParameters&&) = delete;
	BindlessParameters& operator=(BindlessParameters&&) = delete;

	template<class TData>
	inline static uint32_t AddRange(TData &&data)
	{
		// Copy data to heap and store void pointer -> We don't care about the type about that point.
		size_t dataSize = sizeof(TData);
		auto *bytes = new TData;
		*bytes = data;

		// Add range
		uint32_t currentOffset = m_LastOffset;
		m_Ranges.push_back({ currentOffset, dataSize, bytes });

		// Pad the data size to minimum alignment
		// and move the offset
		m_LastOffset += PadSizeToMinAlignment(dataSize);
		return currentOffset;
	}

	static void Build(VkDevice device);

	[[nodiscard]] static VkDescriptorSet GetDescriptorSet();
	[[nodiscard]] static VkDescriptorSetLayout GetDescriptorSetLayout();

	//Needs to be set before doing anything else (Check the GPU limits)
	inline static uint32_t MinimumUniformBufferPadding;

private:
	//The PadSizeToMinAlignment returns the size that is a multiple of MinimumUniformBufferPadding.
	static uint32_t PadSizeToMinAlignment(uint32_t originalSize);

	inline static uint32_t m_LastOffset{};
	inline static std::vector<Range> m_Ranges;
	inline static VkDescriptorSetLayout m_Layout;
	inline static VkDescriptorSet m_DescriptorSet;
	inline static VmaAllocation m_Allocation;
	inline static VkBuffer m_Buffer;
};