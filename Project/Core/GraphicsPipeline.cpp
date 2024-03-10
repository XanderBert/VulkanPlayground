#include "GraphicsPipeline.h"
#include "../shaders/Shader.h"

void GraphicsPipeline::SetShaders(const std::vector<VkPipelineShaderStageCreateInfo>& shaders,	const VkRenderPass& renderPass, const VkDevice& device)
{
	//destroy previous pipeline layout and graphics pipeline
	Cleanup(device);

	//TODO: Move out of this
	//TOOD THis gets out of scope when next call gets made so i need to store it somewhere
	//m_LastUsedShaders = shaders;
	//m_RenderPass = renderPass;
	//m_Device = device;


	CreatePipelineLayout(device);

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
	pipelineInfo.stageCount = shaders.size();
	pipelineInfo.pStages = shaders.data();
	pipelineInfo.pVertexInputState = &Shader::CreateVertexInputStateInfo();
	pipelineInfo.pInputAssemblyState = &Shader::CreateInputAssemblyStateInfo();
	pipelineInfo.pViewportState = &CreateViewportState();
	pipelineInfo.pRasterizationState = &CreateRasterizer();
	pipelineInfo.pMultisampleState = &CreateMultisampling();
	pipelineInfo.pColorBlendState = &CreateColorBlending(colorBlendAttachment);
	pipelineInfo.pDynamicState = &CreateDynamicState(dynamicStates);
	pipelineInfo.layout = m_PipelineLayout;
	pipelineInfo.renderPass = renderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

	if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_GraphicsPipeline) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create graphics pipeline!");
	}
}
