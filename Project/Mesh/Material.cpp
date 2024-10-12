#include "Material.h"
#include "Core/GlobalDescriptor.h"
#include "Core/SwapChain.h"

#include "Timer/GameTimer.h"
#include "shaders/Logic/Shader.h"
#include "vulkanbase/VulkanTypes.h"


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
}

void Material::OnImGui()
{
	ImGui::Text("Shader Count: %d", static_cast<int>(m_Shaders.size()));

	for (const auto& shader : m_Shaders)
	{
		shader->OnImGui(m_MaterialName);
	}

    m_DescriptorSet.OnImGui();
}

void Material::Bind(const VkCommandBuffer commandBuffer, const glm::mat4x4 &pushConstantMatrix) {

    // Update model matrix
    m_pGraphicsPipeline->BindPushConstant(commandBuffer, pushConstantMatrix);

    //TODO: maybe create a different "material" for depth and compute shaders


    m_pGraphicsPipeline->BindPipeline(commandBuffer, m_PipelineType);
    m_DescriptorSet.Bind(m_pContext, commandBuffer, m_pGraphicsPipeline->GetPipelineLayout(), 1, m_PipelineType);

    // TODO: This would be a better solution:
    //  for each material
    //  {
    //      bind material resources  // material parameters and textures
    //      for each mesh
    //      {
    //          bind mesh resources  // object transforms
    //          draw mesh
    //      }
    //  }

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

	m_Shaders.push_back(ShaderManager::CreateShader(m_pContext, shaderPath, shaderType, this));
	return m_Shaders.back();
}

std::vector<Shader *> Material::GetShaders() const
{
    return m_Shaders;
}

const VkPipelineLayout &Material::GetPipelineLayout() const
{
    return m_pGraphicsPipeline->GetPipelineLayout();
}

VkPipelineLayoutCreateInfo Material::GetPipelineLayoutCreateInfo() {
    // Push Constant in the Vertex Shader for model
    static VkPushConstantRange pushConstantRange{};
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(glm::mat4x4);
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_COMPUTE_BIT;

    m_SetLayouts =
    {
            GlobalDescriptor::GetLayout(),
            m_DescriptorSet.GetLayout(m_pContext),
    };

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(m_SetLayouts.size());
    pipelineLayoutInfo.pSetLayouts = m_SetLayouts.data();
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
    pipelineLayoutInfo.pushConstantRangeCount = 1;

    return pipelineLayoutInfo;
}
std::string Material::GetMaterialName() const { return m_MaterialName; }
DescriptorSet *Material::GetDescriptorSet() { return &m_DescriptorSet; }
VkCullModeFlags Material::GetCullModeBit() const {
    return m_CullMode;
}
void Material::SetCullMode(VkCullModeFlags cullMode) {
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

void Material::SetDepthOnly(bool depthOnly)
{
    m_IsDepthOnly = depthOnly;
}

void Material::CreatePipeline()
{
	m_pGraphicsPipeline->CreatePipeline(m_pContext, this);
}
