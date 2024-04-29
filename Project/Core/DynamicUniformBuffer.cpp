#include "DynamicUniformBuffer.h"

#include <algorithm>
#include <glm/vec4.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vulkanbase/VulkanTypes.h>

#include "Buffer.h"
#include "Descriptor.h"


//---------------------------------------------------------------
//-------------------------DynamicBuffer-------------------------
//---------------------------------------------------------------
void DynamicBuffer::Init(VulkanContext* vulkanContext)
{
	//Pad the data to 256 bytes
	//const size_t padding = 256 - (m_Data.size() % 256);
	//m_Data.insert(m_Data.end(), padding, 0);


	//Log the size of the buffer in bytes
	LogInfo("Dynamic buffer size: " + std::to_string(GetSize()) + " bytes");

	Core::Buffer::CreateBuffer(vulkanContext, GetSize(), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_UniformBuffer, m_UniformBuffersMemory);
	vkMapMemory(vulkanContext->device, m_UniformBuffersMemory, 0, GetSize(), 0, &m_UniformBuffersMapped);
}

void DynamicBuffer::ProperBind(int bindingNumber, Descriptor::DescriptorWriter &descriptorWriter) const {
    //Update the data for the descriptor set
    memcpy(m_UniformBuffersMapped, GetData(), GetSize());

    //Write the buffer to the descriptor set
    descriptorWriter.WriteBuffer(bindingNumber, m_UniformBuffer, GetSize(), 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
}
void DynamicBuffer::FullRebind(int bindingNumber, const VkDescriptorSet &descriptorSet, Descriptor::DescriptorWriter &descriptorWriter, VulkanContext *vulkanContext) const
{
    memcpy(m_UniformBuffersMapped, GetData(), GetSize());

    descriptorWriter.Cleanup();
    descriptorWriter.WriteBuffer(bindingNumber, m_UniformBuffer, GetSize(), 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    descriptorWriter.UpdateSet(vulkanContext->device, descriptorSet);
}


void DynamicBuffer::Cleanup(VkDevice device) const
{
	vkDestroyBuffer(device, m_UniformBuffer, nullptr);
	vkFreeMemory(device, m_UniformBuffersMemory, nullptr);
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
void DynamicBuffer::OnImGui() const {
    ImGui::Text("Uniform Buffer Size: %d bytes", GetSize());
    ImGui::Text("Data: ");

    //Display data in rows, each row has 4 floats
    for(int i {}; i <= m_Data.size() - 4; ++i)
    {
        ImGui::Text("%f %f %f %f", m_Data[i], m_Data[i + 1], m_Data[i + 2], m_Data[i + 3]);
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