#include "vulkanbase/VulkanBase.h"
#include <Core/GraphicsPipeline.h>

#include "Core/RenderPass.h"
#include "shaders/Shader.h"


//Material class
void VulkanBase::createGraphicsPipeline()
{
	const VkDevice device = m_pContext->device;

	const VkPipelineShaderStageCreateInfo vertShaderStageInfo = Shader::CreateShaderInfo(device, VK_SHADER_STAGE_VERTEX_BIT, "shader.vert");
	const VkPipelineShaderStageCreateInfo fragShaderStageInfo = Shader::CreateShaderInfo(device, VK_SHADER_STAGE_FRAGMENT_BIT, "shader.frag");

	const std::vector shaderStages = 
	{
		vertShaderStageInfo,
		fragShaderStageInfo
	};


	m_pGraphicsPipeline.CreatePipeline(device, swapChainImageFormat, shaderStages);


	vkDestroyShaderModule(device, vertShaderStageInfo.module, nullptr);
	vkDestroyShaderModule(device, fragShaderStageInfo.module, nullptr);
}
