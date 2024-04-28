#include "Material.h"
#include "vulkanbase/VulkanTypes.h"
#include "Core/Descriptor.h"
#include <shaders/Logic/Shader.h>

#include <cmath>
#include "MaterialManager.h"
#include "Timer/GameTimer.h"



void Material::CleanUp()
{
    m_DescriptorSet.CleanUp(m_pContext->device);
    m_pGraphicsPipeline->Cleanup(m_pContext->device);
}

Material::Material(VulkanContext *vulkanContext, std::string materialName)
    : m_pContext(vulkanContext)
    , m_MaterialName(std::move(materialName))
{
    m_Shaders.reserve(2);
    m_pGraphicsPipeline = std::make_unique<GraphicsPipeline>();

    m_pContext = vulkanContext;

    const auto ubo = m_DescriptorSet.AddUniformBuffer(0);
    ubo->AddVariable(glm::vec4{1});

    //m_DescriptorSet.AddTexture(1, "texture.jpg",m_pContext);
}

void Material::OnImGui() const
{
	ImGui::Text("Shader Count: %d", static_cast<int>(m_Shaders.size()));

	for (const auto& shader : m_Shaders)
	{
		shader->OnImGui(m_MaterialName);
	}

    m_DescriptorSet.OnImGui();
}

void Material::Bind(const VkCommandBuffer commandBuffer, const glm::mat4x4& pushConstantMatrix)
{
    //Update model matrix
    m_pGraphicsPipeline->BindPushConstant(commandBuffer, pushConstantMatrix);


    //TODO: This is a temporary solution to avoid binding the same pipeline multiple times
    //This would be a better solution:
    // for each material
    // {
    //     bind material resources  // material parameters and textures
    //     for each mesh
    //     {
    //         bind mesh resources  // object transforms
    //         draw mesh
    //     }
    // }

    //Don't bind the same pipeline if it's already bound
    //if(MaterialManager::GetCurrentBoundPipeline() ==  m_pGraphicsPipeline.get()) return;
    //MaterialManager::SetCurrentBoundPipeline(m_pGraphicsPipeline.get());

    //Bind the pipeline
    m_pGraphicsPipeline->BindPipeline(commandBuffer);

    GlobalDescriptor::Bind(m_pContext, commandBuffer, m_pGraphicsPipeline->GetPipelineLayout());

    //Update the material uniform buffer for testing
    const float time = GameTimer::GetElapsedTime();
    const float changingValue = std::sin(time) * 0.5f + 0.5f;
    m_DescriptorSet.GetUniformBuffer(0)->UpdateVariable(0, glm::vec4{changingValue});

    m_DescriptorSet.Bind(m_pContext, commandBuffer, m_pGraphicsPipeline->GetPipelineLayout(), 1);
}

Shader* Material::AddShader(const std::string& shaderPath, const ShaderType shaderType)
{
	m_Shaders.push_back(ShaderManager::CreateShader(m_pContext, shaderPath, shaderType, this));
	return m_Shaders.back();
}

void Material::ReloadShaders(Shader *shader)
{
    m_pGraphicsPipeline->CreatePipeline(m_pContext, this);
}

std::vector<Shader *> Material::GetShaders() const
{
    return m_Shaders;
}

const VkPipelineLayout &Material::GetPipelineLayout() const
{
    return m_pGraphicsPipeline->GetPipelineLayout();
}

VkPipelineLayoutCreateInfo Material::GetPipelineLayoutCreateInfo()
{
    // Push Constant in the Vertex Shader for model
    static VkPushConstantRange pushConstantRange{};
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(glm::mat4x4);
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    m_SetLayouts = {GlobalDescriptor::GetLayout(), m_DescriptorSet.GetLayout(m_pContext), };

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = m_SetLayouts.size();
    pipelineLayoutInfo.pSetLayouts = m_SetLayouts.data();
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
    pipelineLayoutInfo.pushConstantRangeCount = 1;

    return pipelineLayoutInfo;
}

void Material::CreatePipeline()
{
	m_pGraphicsPipeline->CreatePipeline(m_pContext, this);
}
