#include "Material.h"

#include "vulkanbase/VulkanTypes.h"
#include "Core/Descriptor.h"
#include "Core/Buffer.h"
#include <shaders/Shader.h>


void Material::CleanUp() const
{
	vkDestroyBuffer(m_pContext->device, m_UniformBuffer, nullptr);
	vkFreeMemory(m_pContext->device, m_UniformBuffersMemory, nullptr);

	m_pDescriptorAllocator->Cleanup();
	m_pDescriptorCache->Cleanup();

	m_pGraphicsPipeline->Cleanup(m_pContext->device);
}

void Material::Bind(VkCommandBuffer commandBuffer) const
{
	m_pGraphicsPipeline->BindPipeline(commandBuffer);
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pGraphicsPipeline->GetPipelineLayout(), 0, 1, &m_DescriptorSet, 0, nullptr);

	memcpy(m_UniformBuffersMapped, m_pDynamicBufferObject.GetData(), m_pDynamicBufferObject.GetSize());
}

void Material::AddShader(const std::string& shaderPath, ShaderType shaderType)
{
	m_Shaders.push_back(Shader::CreateShaderInfo(m_pContext->device, static_cast<VkShaderStageFlagBits>(shaderType), shaderPath));
}

uint16_t Material::AddShaderVariable(const glm::mat4& matrix)
{
	return m_pDynamicBufferObject.AddMatrix(matrix);
}

void Material::UpdateShaderVariable(uint16_t handle, glm::mat4& matrix)
{
	m_pDynamicBufferObject.UpdateMatrix(handle, matrix);
}

void Material::CreatePipeline()
{
	const VkDeviceSize bufferSize = m_pDynamicBufferObject.GetSize();
	Core::Buffer::CreateBuffer(m_pContext, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_UniformBuffer, m_UniformBuffersMemory);
	vkMapMemory(m_pContext->device, m_UniformBuffersMemory, 0, bufferSize, 0, &m_UniformBuffersMapped);

	VkDescriptorBufferInfo bufferInfo{};
	bufferInfo.buffer = m_UniformBuffer;
	bufferInfo.offset = 0;
	bufferInfo.range = m_pDynamicBufferObject.GetSize();

	m_DescriptorBuilder.BindBuffer(0, &bufferInfo, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT).Build(m_DescriptorSet, m_DescriptorSetLayout);

	m_pGraphicsPipeline->CreatePipeline(m_pContext->device, m_pContext->swapChainImageFormat, m_Shaders, m_DescriptorSetLayout);

	for (const auto& shader : m_Shaders)
	{
		vkDestroyShaderModule(m_pContext->device, shader.module, nullptr);
	}
}
