#include "GraphicsPipeline.h"

#include "DepthResource.h"
#include "../shaders/Shader.h"
#include "Descriptor.h"

void GraphicsPipeline::CreatePipeline(const VulkanContext* vulkanContext, const std::vector<VkPipelineShaderStageCreateInfo>& shaders, VkDescriptorSetLayout descriptorSetLayout)
{
	SetShaders(shaders);
	GraphicsPipelineBuilder::CreatePipeline(*this, vulkanContext, descriptorSetLayout);
}

void GraphicsPipeline::BindPushConstant(const VkCommandBuffer commandBuffer, const glm::mat4x4& matrix) const
{
	vkCmdPushConstants(commandBuffer, m_PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4x4), &matrix);
}

void GraphicsPipeline::BindPipeline(const VkCommandBuffer& commandBuffer) const
{
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline);
}

void GraphicsPipelineBuilder::CreatePipeline(GraphicsPipeline& graphicsPipeline, const VulkanContext* vulkanContext, VkDescriptorSetLayout descriptorSetLayout)
{
	//Check if the graphics pipeline has shaders
	LogAssert(graphicsPipeline.HasShaders(), "The Graphicspipeline doesn't have shaders", true)

	//destroy previous pipeline layout and graphics pipeline
	graphicsPipeline.Cleanup(vulkanContext->device);


	//Create dynamic rendering structure
	VkPipelineRenderingCreateInfoKHR pipelineRenderingCreateInfo{};
	pipelineRenderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
	pipelineRenderingCreateInfo.colorAttachmentCount = 1;
	pipelineRenderingCreateInfo.pColorAttachmentFormats = &vulkanContext->swapChainImageFormat;
	pipelineRenderingCreateInfo.depthAttachmentFormat = DepthResource::DepthResource::GetFormat();

	CreatePipelineLayout(vulkanContext->device, graphicsPipeline.m_PipelineLayout, descriptorSetLayout);

	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;

	const std::vector<VkDynamicState> dynamicStates =
	{
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};

	VkGraphicsPipelineCreateInfo pipelineInfo{ VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
	pipelineInfo.pNext = &pipelineRenderingCreateInfo;
	pipelineInfo.renderPass = VK_NULL_HANDLE;
	pipelineInfo.pInputAssemblyState = &Shader::CreateInputAssemblyStateInfo();
	pipelineInfo.pRasterizationState = &CreateRasterizer();
	pipelineInfo.pColorBlendState = &CreateColorBlending(colorBlendAttachment);
	pipelineInfo.pMultisampleState = &CreateMultisampling();
	pipelineInfo.pViewportState = &CreateViewportState();
	pipelineInfo.pDepthStencilState = &DepthResource::DepthResource::GetDepthPipelineInfo(VK_TRUE, VK_TRUE);
	pipelineInfo.pDynamicState = &CreateDynamicState(dynamicStates);
	pipelineInfo.pVertexInputState = &Shader::CreateVertexInputStateInfo();
	pipelineInfo.stageCount = graphicsPipeline.m_ActiveShaders.size();
	pipelineInfo.pStages = graphicsPipeline.m_ActiveShaders.data();
	pipelineInfo.layout = graphicsPipeline.m_PipelineLayout;

	VulkanCheck(vkCreateGraphicsPipelines(vulkanContext->device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline.m_GraphicsPipeline), "Failed to create graphics pipeline!")
}

void GraphicsPipelineBuilder::CreatePipelineLayout(const VkDevice& device, VkPipelineLayout& pipelineLayout, VkDescriptorSetLayout descriptorSetLayout)
{
	VkPushConstantRange pushConstant{};
	pushConstant.offset = 0;
	pushConstant.size = sizeof(glm::mat4x4);
	pushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};

	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
	pipelineLayoutInfo.pPushConstantRanges = &pushConstant;
	pipelineLayoutInfo.pushConstantRangeCount = 1;

	VulkanCheck(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout), "Failed To Create PipelineLayout")
}