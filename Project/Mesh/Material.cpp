#include "Material.h"
#include "Core/GlobalDescriptor.h"
#include "Core/SwapChain.h"

#include "shaders/Logic/Shader.h"
#include "vulkanbase/VulkanTypes.h"



void Material::CleanupPipeline() const
{
    vkDestroyPipeline(m_pContext->device, m_GraphicsPipeline, nullptr);
}

Material::Material(VulkanContext *vulkanContext, std::string materialName)
    : m_pContext(vulkanContext)
    , m_MaterialName(std::move(materialName))
{
    m_Shaders.reserve(2);
    m_pContext = vulkanContext;
}

void Material::OnImGui()
{
	ImGui::Text("Shader Count: %d", static_cast<int>(m_Shaders.size()));

	for (const auto& shader : m_Shaders)
	{
		shader->OnImGui(m_MaterialName);
	}

    //m_DescriptorSet.OnImGui();
}

void Material::Bind(VkCommandBuffer commandBuffer) const
{
	//Bind the pipeline
    vkCmdBindPipeline(commandBuffer, static_cast<VkPipelineBindPoint>(m_PipelineType), m_GraphicsPipeline);

	//TODO: I want it to move out of here
	//Maybe in a struct where the indices are stored?
	//When i do this this material is locked to a specific amount of indices
	//Bind the indices for this material
	vkCmdBindDescriptorSets(commandBuffer,  bindPoint,  pipelineLayout,  1, &bindlessParams.getDescriptorSet(), 1, &rangePBR);
}

Shader *Material::SetShader(const std::string &shaderPath, ShaderType shaderType)
{
    //Check if a shader of this type already exists
    for (auto& shader : m_Shaders)
    {
        if (shader->GetShaderType() == shaderType)
        {
            ShaderManager::RemoveMaterial(shader, this);
            std::erase(m_Shaders, shader);
            break;
        }
    }

    return AddShader(shaderPath, shaderType);
}

Shader * Material::AddShader(const std::string& shaderPath, const ShaderType shaderType)
{
    if(shaderType == ShaderType::ComputeShader)
    {
        m_PipelineType = PipelineType::Compute;
    }

	m_Shaders.emplace_back(ShaderManager::CreateShader(m_pContext, shaderPath, shaderType, this));
	return m_Shaders.back();
}

const std::vector<Shader*>& Material::GetShaders() const
{
    return m_Shaders;
}

std::string Material::GetMaterialName() const
{
	return m_MaterialName;
}

VkCullModeFlags Material::GetCullModeBit() const
{
    return m_CullMode;
}

void Material::SetCullMode(VkCullModeFlags cullMode)
{
    m_CullMode = cullMode;
}

bool Material::GetDepthOnly() const
{
    return m_IsDepthOnly;
}

bool Material::IsCompute() const
{
    return m_PipelineType == PipelineType::Compute;
}

bool Material::IsSSAO() const
{
	return m_IsSSAO;
}

bool Material::IsComposite() const
{
	return m_IsComposite;
}

void Material::SetIsComposite(bool isComposite)
{
	m_IsComposite = isComposite;
}

void Material::SetDepthOnly(bool depthOnly)
{
    m_IsDepthOnly = depthOnly;
}

void Material::SetSSAOPass(bool isSSAO)
{
	m_IsSSAO = isSSAO;
}

void Material::CreatePipeline()
{
    GraphicsPipelineBuilder::CreatePipeline(m_pContext, this);
}
