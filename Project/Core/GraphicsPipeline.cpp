#include "GraphicsPipeline.h"
#include "../shaders/Shader.h"

void GraphicsPipeline::CreatePipeline(const VkDevice& device, const VkFormat& swapChainImageFormat, const std::vector<VkPipelineShaderStageCreateInfo>& shaders)
{
	//TODO: Cleanup the Pipeline if it already existed and Check if the RenderPass should be deleted/created 
	RenderPass::CreateRenderPass(device, swapChainImageFormat, m_RenderPass);
	SetShaders(shaders);
	GraphicsPipelineBuilder::CreatePipeline(*this, device);
}

void GraphicsPipelineBuilder::CreatePipeline(GraphicsPipeline& graphicsPipeline, const VkDevice& device)
{
	//Check if ShadersStages are set
	if(!graphicsPipeline.IsReadyForInitialization())
	{
		throw std::runtime_error("GraphicsPipelineBuilder::CreatePipeline()");
	}

	//destroy previous pipeline layout and graphics pipeline
	//graphicsPipeline.Cleanup(device);

	CreatePipelineLayout(device, graphicsPipeline.m_PipelineLayout);

	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;

	const std::vector<VkDynamicState> dynamicStates =
	{
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};


	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = graphicsPipeline.m_ActiveShaders.size();
	pipelineInfo.pStages = graphicsPipeline.m_ActiveShaders.data();
	pipelineInfo.pVertexInputState = &Shader::CreateVertexInputStateInfo();
	pipelineInfo.pInputAssemblyState = &Shader::CreateInputAssemblyStateInfo();
	pipelineInfo.pViewportState = &CreateViewportState();
	pipelineInfo.pRasterizationState = &CreateRasterizer();
	pipelineInfo.pMultisampleState = &CreateMultisampling();
	pipelineInfo.pColorBlendState = &CreateColorBlending(colorBlendAttachment);
	pipelineInfo.pDynamicState = &CreateDynamicState(dynamicStates);
	pipelineInfo.layout = graphicsPipeline.m_PipelineLayout;
	pipelineInfo.renderPass = graphicsPipeline.m_RenderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

	if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline.m_GraphicsPipeline) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create graphics pipeline!");
	}
}
