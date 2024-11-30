#include "DynamicUniformBuffer.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <algorithm>
#include <glm/vec4.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/type_aligned.hpp>

#include <vulkanbase/VulkanTypes.h>

#include "Buffer.h"
#include "Descriptor.h"
#include "DescriptorSet.h"
#include "Core/VmaUsage.h"
#include "Patterns/ServiceLocator.h"



//---------------------------------------------------------------
//-------------------------DynamicBuffer-------------------------
//---------------------------------------------------------------
DynamicBuffer::DynamicBuffer(DynamicBuffer &&other) noexcept {
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


void DynamicBuffer::Write(Descriptor::DescriptorWriter &writer, DescriptorResourceHandle newIndex, DescriptorType descriptorType)
{
	//Log the size of the buffer in bytes
	LogInfo("Initializing Dynamic buffer with size: " + std::to_string(GetSize()) + " bytes");

	//Get the bufferType
	BufferType bufferType = descriptorType == DescriptorType::UniformBuffer ? BufferType::UniformBuffer : BufferType::StorageBuffer;

	//Create the buffer
	Core::Buffer::CreateBuffer(GetSize(), static_cast<VkBufferUsageFlags>(bufferType), m_UniformBuffer, m_UniformBuffersMemory, true, true);

	//Allocate memory on the GPU
	VmaAllocationInfo allocInfo;
	vmaGetAllocationInfo(Allocator::vmaAllocator, m_UniformBuffersMemory, &allocInfo);
	m_UniformBuffersMapped = (void*)allocInfo.pMappedData;

	//Write the buffer
	writer.WriteBuffer(newIndex, m_UniformBuffer, GetSize(), 0, descriptorType);
}

void DynamicBuffer::Bind() const
{
	//Update the data for the descriptor set
    memcpy(m_UniformBuffersMapped, GetData(), GetSize());
}

void DynamicBuffer::Cleanup(VkDevice device) const
{
    vmaDestroyBuffer(Allocator::vmaAllocator, m_UniformBuffer, m_UniformBuffersMemory);
}

uint16_t DynamicBuffer::AddVariable(const float value)
{
	constexpr uint8_t size = sizeof(float) / sizeof(float);
    return Insert(&value, size);
}

uint16_t DynamicBuffer::AddVariable(const glm::vec2 &value)
{
	constexpr uint8_t size = sizeof(glm::vec2) / sizeof(float);
	return Insert(glm::value_ptr(value), size);
}

uint16_t DynamicBuffer::AddVariable(const glm::vec4& value)
{
	constexpr uint8_t size = sizeof(glm::vec4) / sizeof(float);
	return Insert(glm::value_ptr(value), size);
}

uint16_t DynamicBuffer::AddVariable(const glm::mat4& value)
{
	constexpr uint8_t size = sizeof(glm::mat4) / sizeof(float);
	return Insert(glm::value_ptr(value), size);
}

void DynamicBuffer::UpdateVariable(uint16_t handle, const float value)
{
	constexpr uint8_t size = sizeof(float) / sizeof(float);
	Update(handle, &value, size);
}

void DynamicBuffer::UpdateVariable(uint16_t handle, const glm::vec2 &value)
{
	constexpr uint8_t size = sizeof(glm::vec2) / sizeof(float);
	Update(handle, glm::value_ptr(value), size);
}

void DynamicBuffer::UpdateVariable(uint16_t handle, const glm::vec4 &value) {
    constexpr uint8_t size = sizeof(glm::vec4) / sizeof(float);
    Update(handle, glm::value_ptr(value), size);
}
void DynamicBuffer::OnImGui()
{
    std::string labelAddition = "##" + std::to_string(reinterpret_cast<uintptr_t>(this));

    ImGui::Text("Uniform Buffer Size: %d bytes", GetSize());
    ImGui::Text("Data: ");
    //Display data in rows, each row has 4 floats
    for(int i {}; i <= m_Data.size() - 4; i += 4)
    {
        const std::string label = std::to_string(i) + labelAddition;

        //Get pointer to Those 4 floats
        float* dataPtr = m_Data.data() + i;
        ImGui::ColorEdit4(label.c_str(), dataPtr);
    }

    std::string labelAddColor4 = "Add Color4" + labelAddition;
    if(ImGui::Button(labelAddColor4.c_str()))
    {
        AddVariable(glm::vec4{0});
        Cleanup(ServiceLocator::GetService<VulkanContext>()->device);
    }

    std::string labelAddMat4 = "Add Mat4" + labelAddition;
    if(ImGui::Button(labelAddMat4.c_str()))
    {
        AddVariable(glm::mat4{1});
        Cleanup(ServiceLocator::GetService<VulkanContext>()->device);
    }
}
void DynamicBuffer::UpdateVariable(uint16_t handle, const glm::mat4& value)
{
	constexpr uint8_t size = sizeof(glm::mat4) / sizeof(float);
	Update(handle, glm::value_ptr(value), size);
}

const float* DynamicBuffer::GetData() const
{
	return m_Data.data();
}

size_t DynamicBuffer::GetSize() const
{
	return m_Data.size() * sizeof(float);
}

uint16_t DynamicBuffer::Insert(const float* dataPtr, uint8_t size)
{
	//Check if the size is a multiple of vec4
	LogAssert(size % 4 == 0, "Size is not a multiple of 4", true)

	m_Data.insert(m_Data.end(), dataPtr, dataPtr + size);

	return m_Data.size() - size;
}

void DynamicBuffer::Update(uint16_t handle, const float* dataPtr, uint8_t size)
{
	LogAssert(handle + size <= m_Data.size(), "Handle out of bounds", true)

	std::copy_n(dataPtr, size, m_Data.begin() + handle);
}

void BindlessParameters::Build(VkDevice device)
{
	//Create the buffer
	Core::Buffer::CreateBuffer(m_LastOffset, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, m_Buffer, m_Allocation, true, false);

	// Copy the ranges to the GPU
	uint8_t *data = nullptr;
	vmaMapMemory(Allocator::vmaAllocator, m_Allocation, reinterpret_cast<void**>(&data));
	for (const auto &range : m_Ranges)
	{
		memcpy(data + range.offset, range.data, range.size);
	}
	vmaUnmapMemory(Allocator::vmaAllocator, m_Allocation);

	// Create layout for descriptor set
	VkDescriptorSetLayoutBinding binding{};
	binding.binding = Descriptor::ParametersUboBinding;
	binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	binding.descriptorCount = 1;
	binding.stageFlags = VK_SHADER_STAGE_ALL;

    VkDescriptorSetLayoutCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    createInfo.bindingCount = 1;
    createInfo.pBindings = &binding;
    vkCreateDescriptorSetLayout(device, &createInfo, nullptr, &m_Layout);

	// Get maximum size of a single range
	uint32_t maxRangeSize = 0;
	for (auto &range : m_Ranges)
	{
		maxRangeSize = std::max(range.size, maxRangeSize);
	}

	m_DescriptorSet = Descriptor::DescriptorManager::Allocate(device, m_Layout);

	//Write the buffer
	VkDescriptorBufferInfo bufferInfo{};
	bufferInfo.buffer = m_Buffer;
	bufferInfo.offset = 0;
	bufferInfo.range = maxRangeSize;

	VkWriteDescriptorSet write{};
	write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	write.dstBinding = 0;
	write.dstSet = m_DescriptorSet;
	write.descriptorCount = 1;
	write.dstArrayElement = 0;
	write.pBufferInfo = &bufferInfo;
	vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);
}

VkDescriptorSet BindlessParameters::GetDescriptorSet()
{
	return m_DescriptorSet;
}

VkDescriptorSetLayout BindlessParameters::GetDescriptorSetLayout()
{
	return m_Layout;
}

uint32_t BindlessParameters::PadSizeToMinAlignment(uint32_t originalSize)
{
	return (originalSize + MinimumUniformBufferPadding - 1) & ~(MinimumUniformBufferPadding - 1);
}
