#include "DynamicUniformBuffer.h"

#include <algorithm>
#include <glm/vec4.hpp>
#include <glm/gtc/type_ptr.hpp>
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
        m_BufferType = other.m_BufferType;
        m_DescriptorType = other.m_DescriptorType;

        other.m_UniformBuffer = nullptr;
        other.m_UniformBuffersMemory = nullptr;
        other.m_UniformBuffersMapped = nullptr;
    }
}



void DynamicBuffer::Init()
{
	//Log the size of the buffer in bytes
	LogInfo("Initializing Dynamic buffer with size: " + std::to_string(GetSize()) + " bytes");

	Core::Buffer::CreateBuffer(GetSize(), static_cast<VkBufferUsageFlags>(m_BufferType), m_UniformBuffer, m_UniformBuffersMemory, true, true);

    VmaAllocationInfo allocInfo;
    vmaGetAllocationInfo(Allocator::VmaAllocator, m_UniformBuffersMemory, &allocInfo);
    m_UniformBuffersMapped = (void*)allocInfo.pMappedData;
}

void DynamicBuffer::ProperBind(int bindingNumber, Descriptor::DescriptorWriter &descriptorWriter) const {
    //Update the data for the descriptor set
    memcpy(m_UniformBuffersMapped, GetData(), GetSize());

    //Write the buffer to the descriptor set
    descriptorWriter.WriteBuffer(bindingNumber, m_UniformBuffer, GetSize(), 0, static_cast<VkDescriptorType>(m_DescriptorType));
}
void DynamicBuffer::FullRebind(int bindingNumber, const VkDescriptorSet &descriptorSet, Descriptor::DescriptorWriter &descriptorWriter, VulkanContext *vulkanContext) const
{
    memcpy(m_UniformBuffersMapped, GetData(), GetSize());

    descriptorWriter.Cleanup();
    descriptorWriter.WriteBuffer(bindingNumber, m_UniformBuffer, GetSize(), 0, static_cast<VkDescriptorType>(m_DescriptorType));
    descriptorWriter.UpdateSet(vulkanContext->device, descriptorSet);
}


void DynamicBuffer::Cleanup(VkDevice device) const
{
    vmaDestroyBuffer(Allocator::VmaAllocator, m_UniformBuffer, m_UniformBuffersMemory);
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
        Init();
    }

    std::string labelAddMat4 = "Add Mat4" + labelAddition;
    if(ImGui::Button(labelAddMat4.c_str()))
    {
        AddVariable(glm::mat4{1});
        Cleanup(ServiceLocator::GetService<VulkanContext>()->device);
        Init();
    }
}

void DynamicBuffer::SetDescriptorType(DescriptorType descriptorType)
{
    m_DescriptorType = descriptorType;
    m_BufferType = m_DescriptorType == DescriptorType::UniformBuffer ? BufferType::UniformBuffer : BufferType::StorageBuffer;
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