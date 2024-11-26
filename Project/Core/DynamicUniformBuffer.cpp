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



void DynamicBuffer::Init(BufferType bufferType)
{
	//Log the size of the buffer in bytes
	LogInfo("Initializing Dynamic buffer with size: " + std::to_string(GetSize()) + " bytes");

	Core::Buffer::CreateBuffer(GetSize(), static_cast<VkBufferUsageFlags>(bufferType), m_UniformBuffer, m_UniformBuffersMemory, true, true);

    VmaAllocationInfo allocInfo;
    vmaGetAllocationInfo(Allocator::vmaAllocator, m_UniformBuffersMemory, &allocInfo);
    m_UniformBuffersMapped = (void*)allocInfo.pMappedData;
}

void DynamicBuffer::Bind(Descriptor::DescriptorWriter &descriptorWriter, DescriptorType descriptorType) const
{
	int binding = Descriptor::k_bindless_buffer_binding;
	if(descriptorType == DescriptorType::StorageBuffer) ++binding;

	//Update the data for the descriptor set
    memcpy(m_UniformBuffersMapped, GetData(), GetSize());

    //Write the buffer to the descriptor set
    descriptorWriter.WriteBuffer(binding, m_UniformBuffer, GetSize(), 0, static_cast<VkDescriptorType>(descriptorType));
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
        Init(BufferType::UniformBuffer);
    }

    std::string labelAddMat4 = "Add Mat4" + labelAddition;
    if(ImGui::Button(labelAddMat4.c_str()))
    {
        AddVariable(glm::mat4{1});
        Cleanup(ServiceLocator::GetService<VulkanContext>()->device);
        Init(BufferType::UniformBuffer);
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