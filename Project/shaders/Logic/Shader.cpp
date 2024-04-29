#include "Shader.h"

#include <ranges>

#include <vector>
#include "Mesh/Material.h"
#include "Patterns/ServiceLocator.h"
#include "SpirvHelper.h"
#include "imgui.h"
#include "vulkanbase/VulkanTypes.h"


//--------------------------------------------------------
//-------------------------Shader-------------------------
//--------------------------------------------------------

void Shader::AddMaterial(Material* material)
{
	m_pMaterials.push_back(material);
}


void Shader::OnImGui(const std::string& materialName)
{
	ImGui::Text(m_FileName.c_str());
	ImGui::Separator();
	const std::string label = "Reload##" + m_FileName + materialName;
	const std::string openLabel = "Edit##" + m_FileName + materialName;


	if(ImGui::Button(openLabel.c_str()))
	{
		tools::OpenFile("shaders\\" + m_FileName);
	}
	ImGui::SameLine();
	if(ImGui::Button(label.c_str()))
	{
		ShaderManager::ReloadShader(ServiceLocator::GetService<VulkanContext>(), m_FileName, static_cast<ShaderType>(m_ShaderInfo.stage));
	}
}

std::string Shader::GetFileName() const
{
	return m_FileName;
}

void Shader::Cleanup(VkDevice device) const
{
	CleanupModule(device);
}

void Shader::CleanupModule(VkDevice device) const
{
	vkDestroyShaderModule(device, m_ShaderInfo.module, nullptr);
}

Shader::Shader(const VkPipelineShaderStageCreateInfo& shaderInfo, Material* material, const std::string& filename)
	: m_ShaderInfo(shaderInfo)
	, m_pMaterials({ material })
	, m_FileName(filename)
{

}

VkPipelineShaderStageCreateInfo Shader::GetStageInfo() const
{
	return m_ShaderInfo;
}




//---------------------------------------------------------------
//-------------------------ShaderManager-------------------------
//---------------------------------------------------------------


VkPipelineShaderStageCreateInfo ShaderManager::ShaderBuilder::CreateShaderInfo(const VkDevice& device, VkShaderStageFlagBits shaderStage, const std::string& fileName)
{
	const std::string fileLocation = "shaders/" + fileName + ".spv";
	const std::vector<char> shaderCode = tools::readFile(fileLocation);

	LogAssert(!shaderCode.empty(), "Failed find the shader at the specified path", true)


	VkPipelineShaderStageCreateInfo shaderStageInfo{};
	shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStageInfo.stage = shaderStage;
	shaderStageInfo.module = CreateShaderModule(device, shaderCode);
	shaderStageInfo.pName = "main";

	return shaderStageInfo;
}

VkShaderModule ShaderManager::ShaderBuilder::CreateShaderModule(const VkDevice& device, const std::vector<char>& code)
{
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	VkShaderModule shaderModule;
	VulkanCheck(vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule), "failed to create shader module!")

	return shaderModule;
}

void ShaderManager::ReloadShader(VulkanContext* vulkanContext, const std::string& fileName, ShaderType shaderType)
{
	LogInfo("Reloading shader: " + fileName);

	const auto it = m_ShaderInfo.find(fileName);
	LogAssert(it != m_ShaderInfo.end(), "Shader does not exist", true)

	Shader* shader = it->second.get();
	shader->CleanupModule(vulkanContext->device);

	const VkPipelineShaderStageCreateInfo shaderInfo = ShaderBuilder::CreateShaderInfo(vulkanContext->device, static_cast<VkShaderStageFlagBits>(shaderType), fileName);
	shader->m_ShaderInfo = shaderInfo;

	//Update the shader for every material
	for (Material* material : shader->m_pMaterials)
	{
		material->ReloadShaders();
	}
}

Shader* ShaderManager::CreateShader(VulkanContext* vulkanContext, const std::string& fileName, ShaderType shaderType, Material* material)
{
	// Check if the shader already exists
    if (const auto it = m_ShaderInfo.find(fileName); it != m_ShaderInfo.end())
	{
		Shader* shader = it->second.get();
		shader->AddMaterial(material);
		return shader;
	}

    SpirvHelper::CompileAndSaveShader(fileName);
	VkPipelineShaderStageCreateInfo shaderInfo = ShaderBuilder::CreateShaderInfo(vulkanContext->device, static_cast<VkShaderStageFlagBits>(shaderType), fileName);


	std::unique_ptr<Shader> shaderPtr = std::make_unique<Shader>(shaderInfo, material, fileName);
	Shader* shader = shaderPtr.get();

	m_ShaderInfo.insert(std::make_pair(fileName, std::move(shaderPtr)));

    // ReSharper disable once CppDFALocalValueEscapesFunction
    return shader;
}

void ShaderManager::Cleanup(VkDevice device)
{
	for (const auto &shader: m_ShaderInfo | std::views::values)
	{
		shader->Cleanup(device);
	}

	m_ShaderInfo.clear();
}

VkPipelineVertexInputStateCreateInfo ShaderManager::GetVertexInputStateInfo()
{
	constexpr VkPipelineVertexInputStateCreateInfo vertexInputInfo
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = &m_VertexInputBindingDescription,
        .vertexAttributeDescriptionCount = static_cast<uint32_t>(m_VertexInputAttributeDescription.size()),
        .pVertexAttributeDescriptions = m_VertexInputAttributeDescription.data()
    };
    
	return vertexInputInfo;
}

VkPipelineInputAssemblyStateCreateInfo ShaderManager::GetInputAssemblyStateInfo()
{
	constexpr VkPipelineInputAssemblyStateCreateInfo inputAssembly
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE
    };

	return inputAssembly;
}