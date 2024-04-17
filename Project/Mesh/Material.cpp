#include "Material.h"

#include "vulkanbase/VulkanTypes.h"
#include "Core/Descriptor.h"
#include "Core/Buffer.h"
#include <shaders/Logic/Shader.h>


void Material::CleanUp() const
{
	m_pGraphicsPipeline->Cleanup(m_pContext->device);
}

void Material::OnImGui()
{
	ImGui::Text("Shader Count: %d", m_Shaders.size());
	//m_GraphicsPipeline->OnImGui();

	for (const auto& shader : m_Shaders)
	{
		shader->OnImGui();
	}
	
}

void Material::Bind(VkCommandBuffer commandBuffer, const glm::mat4x4& pushConstantMatrix)
{
	m_pGraphicsPipeline->BindPushConstant(commandBuffer, pushConstantMatrix);
	m_pGraphicsPipeline->BindPipeline(commandBuffer);

	Descriptor::DescriptorManager::m_GlobalDescriptor.Bind(commandBuffer, m_pGraphicsPipeline->GetPipelineLayout());

	for (const auto& shader : m_Shaders)
	{
		shader->Bind(commandBuffer, m_pGraphicsPipeline->GetPipelineLayout(), this);
	}
}

Shader* Material::AddShader(const std::string& shaderPath, ShaderType shaderType)
{
	m_Shaders.push_back(ShaderManager::CreateShader(m_pContext, shaderPath, shaderType, this));
	return m_Shaders.back();
}

void Material::ReloadShaders(Shader* shader)
{
	//m_pGraphicsPipeline->Cleanup(m_pContext->device);
	m_pGraphicsPipeline->CreatePipeline(m_pContext, this);
}

void Material::CreatePipeline()
{
	for (const auto& shader : m_Shaders)
	{
		shader->AddUniformBuffer(m_pContext, 0, this);
	}

	m_pGraphicsPipeline->CreatePipeline(m_pContext, this);
}
