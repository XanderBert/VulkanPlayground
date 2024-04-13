#include "GraphicsPipeline.h"

#include "DepthResource.h"
#include "../shaders/Shader.h"
#include "Descriptor.h"
#include "Mesh/Material.h"

void GraphicsPipeline::CreatePipeline(const VulkanContext* vulkanContext,Material* material)
{
	SetShaders(material->GetShaders());
	GraphicsPipelineBuilder::CreatePipeline(*this, vulkanContext);
}

void GraphicsPipeline::BindPushConstant(const VkCommandBuffer commandBuffer, const glm::mat4x4& matrix) const
{
	vkCmdPushConstants(commandBuffer, m_PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4x4), &matrix);
}

void GraphicsPipeline::BindPipeline(const VkCommandBuffer& commandBuffer) const
{
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline);
}

void GraphicsPipelineBuilder::CreatePipeline(GraphicsPipeline& graphicsPipeline, const VulkanContext* vulkanContext)
{
	//Check if the graphics pipeline has shaders
	LogAssert(graphicsPipeline.HasShaders(), "The Graphicspipeline doesn't have shaders", true)
	LogAssert(vulkanContext, "VulkanContext is nullptr", true)

	//destroy previous pipeline layout and graphics pipeline
	graphicsPipeline.Cleanup(vulkanContext->device);


	//Create dynamic rendering structure
	VkPipelineRenderingCreateInfoKHR pipelineRenderingCreateInfo{};
	pipelineRenderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
	pipelineRenderingCreateInfo.colorAttachmentCount = 1;
	pipelineRenderingCreateInfo.pColorAttachmentFormats = &vulkanContext->swapChainImageFormat;
	pipelineRenderingCreateInfo.depthAttachmentFormat = DepthResource::DepthResource::GetFormat();


	CreatePipelineLayout(vulkanContext->device, graphicsPipeline.m_PipelineLayout, graphicsPipeline);

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
	pipelineInfo.pInputAssemblyState = &ShaderManager::GetInputAssemblyStateInfo();
	pipelineInfo.pRasterizationState = &CreateRasterizer();
	pipelineInfo.pColorBlendState = &CreateColorBlending(colorBlendAttachment);
	pipelineInfo.pMultisampleState = &CreateMultisampling();
	pipelineInfo.pViewportState = &CreateViewportState();
	pipelineInfo.pDepthStencilState = &DepthResource::DepthResource::GetDepthPipelineInfo(VK_TRUE, VK_TRUE);
	pipelineInfo.pDynamicState = &CreateDynamicState(dynamicStates);
	pipelineInfo.pVertexInputState = &ShaderManager::GetVertexInputStateInfo();
	

	std::vector<VkPipelineShaderStageCreateInfo> shaderStages;

	for (const auto& shader : graphicsPipeline.m_ActiveShaders)
	{
		shaderStages.push_back(shader->GetStageInfo());
	}

	pipelineInfo.stageCount = shaderStages.size();
	pipelineInfo.pStages = shaderStages.data();
	pipelineInfo.layout = graphicsPipeline.m_PipelineLayout;

	VulkanCheck(vkCreateGraphicsPipelines(vulkanContext->device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline.m_GraphicsPipeline), "Failed to create graphics pipeline!")
}

void GraphicsPipelineBuilder::CreatePipelineLayout(const VkDevice& device, VkPipelineLayout& pipelineLayout, const GraphicsPipeline& graphicsPipeline)
{
	//Push Constant in the Vertex Shader
	VkPushConstantRange pushConstant{};
	pushConstant.offset = 0;
	pushConstant.size = sizeof(glm::mat4x4);
	pushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};

	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;


	//std::vector<VkDescriptorSetLayout> descriptorSetLayouts;

	//for (const auto& shader : graphicsPipeline.m_ActiveShaders)
	//{
	//	descriptorSetLayouts.push_back(shader->GetDescriptorSetLayout());
	//}

	//descriptorSetLayouts.push_back(Descriptor::DescriptorManager::m_GlobalDescriptor.GetLayout());

	pipelineLayoutInfo.pSetLayouts = &Descriptor::DescriptorManager::m_GlobalDescriptor.GetLayout();
	pipelineLayoutInfo.pPushConstantRanges = &pushConstant;
	pipelineLayoutInfo.pushConstantRangeCount = 1;

	VulkanCheck(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout), "Failed To Create PipelineLayout")
}