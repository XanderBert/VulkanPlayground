#include "Shader.h"
#include "../Mesh/Vertex.h"

VkPipelineShaderStageCreateInfo Shader::CreateShaderInfo(const VkDevice& device, VkShaderStageFlagBits shaderStage, const std::string& fileName)
{
	const std::string fileLocation = "shaders/" + fileName + ".spv";
	const std::vector<char> shaderCode = readFile(fileLocation);

	VkPipelineShaderStageCreateInfo shaderStageInfo{};
	shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStageInfo.stage = shaderStage;
	shaderStageInfo.module = CreateShaderModule(device, shaderCode);;
	shaderStageInfo.pName = "main";

	return shaderStageInfo;
}

VkPipelineVertexInputStateCreateInfo Shader::CreateVertexInputStateInfo()
{
	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &m_VertexInputBindingDescription;
	vertexInputInfo.vertexAttributeDescriptionCount = m_VertexInputAttributeDescription.size();
	vertexInputInfo.pVertexAttributeDescriptions = m_VertexInputAttributeDescription.data();

	return vertexInputInfo;
}

VkPipelineInputAssemblyStateCreateInfo Shader::CreateInputAssemblyStateInfo()
{
	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;
	return inputAssembly;
}

VkShaderModule Shader::CreateShaderModule(const VkDevice& device, const std::vector<char>& code)
{
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create shader module!");
	}
	
	return shaderModule;
}
