#include "Material.h"
#include "vulkanbase/VulkanTypes.h"
#include "Core/Descriptor.h"
#include <shaders/Logic/Shader.h>

#include <cmath>
#include "MaterialManager.h"
#include "Timer/GameTimer.h"


void Material::CleanUp() const
{
	vkDestroyDescriptorSetLayout(m_pContext->device, m_MaterialDescriptorSetLayout, nullptr);

	m_MaterialUniformBuffer.Cleanup(m_pContext->device);
	m_pGraphicsPipeline->Cleanup(m_pContext->device);
}

void Material::OnImGui()
{
	ImGui::Text("Shader Count: %d", static_cast<int>(m_Shaders.size()));

	for (const auto& shader : m_Shaders)
	{
		shader->OnImGui(m_MaterialName);
	}
}

void Material::Bind(const VkCommandBuffer commandBuffer, const glm::mat4x4& pushConstantMatrix)
{
	//Update model matrix
	m_pGraphicsPipeline->BindPushConstant(commandBuffer, pushConstantMatrix);


	//Don't bind the same pipeline if it's already bound
	if(MaterialManager::GetCurrentBoundPipeline() ==  m_pGraphicsPipeline.get()) return;
	MaterialManager::SetCurrentBoundPipeline(m_pGraphicsPipeline.get());


	//Bind the pipeline
	m_pGraphicsPipeline->BindPipeline(commandBuffer);

	//Bind teh Global Descriptor
	GlobalDescriptor::Bind(m_pContext);

	//Update the material uniform buffer for testing
	const float time = GameTimer::GetElapsedTime();
	const float changingValue = std::sin(time) * 0.5f + 0.5f;
	m_MaterialUniformBuffer.UpdateVariable(0, glm::vec4{0.1f,changingValue,changingValue,1});

	//Allocate the descriptor set
	m_MaterialDescriptorSet = Descriptor::DescriptorManager::Allocate(m_pContext->device, m_MaterialDescriptorSetLayout, 0);

	//Bind the material uniform buffer
	m_MaterialUniformBuffer.ProperBind(0, m_MaterialDescriptorSet, m_MaterialWriter, m_pContext);

	//Bind the descriptor sets
	const VkDescriptorSet sets[] = { GlobalDescriptor::GetDescriptorSet(), m_MaterialDescriptorSet };
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pGraphicsPipeline->GetPipelineLayout(), 0, 2, sets, 0, nullptr);
}

Shader* Material::AddShader(const std::string& shaderPath, const ShaderType shaderType)
{
	m_Shaders.push_back(ShaderManager::CreateShader(m_pContext, shaderPath, shaderType, this));
	return m_Shaders.back();
}

void Material::ReloadShaders(Shader* shader)
{
	m_pGraphicsPipeline->CreatePipeline(m_pContext, this);
}

void Material::CreatePipeline()
{
	m_MaterialUniformBuffer.AddVariable({ 0,0,1,1 });
	m_MaterialUniformBuffer.Init(m_pContext);


	Descriptor::DescriptorBuilder builder{};
	builder.AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
	m_MaterialDescriptorSetLayout = builder.Build(m_pContext->device, VK_SHADER_STAGE_VERTEX_BIT);

	m_pGraphicsPipeline->CreatePipeline(m_pContext, this);
}
