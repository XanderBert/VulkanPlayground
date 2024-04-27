#include "GraphicsPipeline.h"

#include "DepthResource.h"
#include "shaders/Logic/Shader.h"
#include "Descriptor.h"
#include "SwapChain.h"
#include "Mesh/Material.h"

void GraphicsPipeline::CreatePipeline(const VulkanContext* vulkanContext,Material* material)
{
	GraphicsPipelineBuilder::CreatePipeline(*this, vulkanContext, material);
}

void GraphicsPipeline::BindPushConstant(const VkCommandBuffer commandBuffer, const glm::mat4x4& matrix) const
{
	vkCmdPushConstants(commandBuffer, m_PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4x4), &matrix);
}

void GraphicsPipeline::BindPipeline(const VkCommandBuffer& commandBuffer) const
{
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline);
}



void GraphicsPipelineBuilder::CreatePipeline(GraphicsPipeline& graphicsPipeline, const VulkanContext* vulkanContext, Material* material)
{
	//destroy previous pipeline layout and graphics pipeline
	graphicsPipeline.Cleanup(vulkanContext->device);

	const VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = material->GetPipelineLayoutCreateInfo();
	VulkanCheck(vkCreatePipelineLayout(vulkanContext->device, &pipelineLayoutCreateInfo, nullptr, &graphicsPipeline.m_PipelineLayout ), "Failed To Create PipelineLayout")

	//Create dynamic rendering structure
	VkPipelineRenderingCreateInfoKHR pipelineRenderingCreateInfo{};
	pipelineRenderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
	pipelineRenderingCreateInfo.colorAttachmentCount = 1;
	pipelineRenderingCreateInfo.pColorAttachmentFormats = &SwapChain::Format();
	pipelineRenderingCreateInfo.depthAttachmentFormat = DepthResource::GetFormat();


	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;

	const std::vector dynamicStates =
	{
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};

	VkGraphicsPipelineCreateInfo pipelineInfo{ VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
	pipelineInfo.pNext = &pipelineRenderingCreateInfo;
	pipelineInfo.renderPass = VK_NULL_HANDLE;
	pipelineInfo.pInputAssemblyState = &ShaderManager::GetInputAssemblyStateInfo();
	pipelineInfo.pRasterizationState = &CreateRasterizer();
	pipelineInfo.pColorBlendState = &CreateColorBlending(colorBlendAttachment);
	pipelineInfo.pMultisampleState = &CreateMultisampling();
	pipelineInfo.pViewportState = &CreateViewportState();
	pipelineInfo.pDepthStencilState = &DepthResource::GetDepthPipelineInfo(VK_TRUE, VK_TRUE);
	pipelineInfo.pDynamicState = &CreateDynamicState(dynamicStates);
	pipelineInfo.pVertexInputState = &ShaderManager::GetVertexInputStateInfo();
	

	std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
	for (const auto& shader : material->GetShaders())
	{
		shaderStages.push_back(shader->GetStageInfo());
	}

	pipelineInfo.stageCount = shaderStages.size();
	pipelineInfo.pStages = shaderStages.data();
	pipelineInfo.layout = graphicsPipeline.m_PipelineLayout;

	VulkanCheck(vkCreateGraphicsPipelines(vulkanContext->device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline.m_GraphicsPipeline), "Failed to create graphics pipeline!")
}
