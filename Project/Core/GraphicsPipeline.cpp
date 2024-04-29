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
    LogInfo("Layout Created For: " + material->GetMaterialName());

	//Create dynamic rendering structure
	const VkPipelineRenderingCreateInfoKHR pipelineRenderingCreateInfo
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR,
        .colorAttachmentCount = 1,
        .pColorAttachmentFormats = &SwapChain::Format(),
        .depthAttachmentFormat = DepthResource::GetFormat(),
    };


    constexpr VkPipelineRasterizationStateCreateInfo rasterizer
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .cullMode = VK_CULL_MODE_BACK_BIT,
        .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
        .depthBiasEnable = VK_FALSE,
        .lineWidth = 1.0f,
    };


	constexpr VkPipelineColorBlendAttachmentState colorBlendAttachment
    {
        .blendEnable = VK_FALSE,
        .colorWriteMask =  VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
    };
    const VkPipelineColorBlendStateCreateInfo colorBlending
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable = VK_FALSE,
        .logicOp = VK_LOGIC_OP_COPY,
        .attachmentCount = 1,
        .pAttachments = &colorBlendAttachment,
        .blendConstants = { 0.0f, 0.0f, 0.0f, 0.0f }
    };


	const std::vector dynamicStates =
	{
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};

    const VkPipelineDynamicStateCreateInfo dynamicState
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
        .pDynamicStates = dynamicStates.data()
    };

    constexpr VkPipelineViewportStateCreateInfo viewportState
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .scissorCount = 1
    };

    constexpr VkPipelineMultisampleStateCreateInfo multisampling
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        .sampleShadingEnable = VK_FALSE,
    };

    const auto inputAssemblyState = ShaderManager::GetInputAssemblyStateInfo();
    const auto depthStencilState = DepthResource::GetDepthPipelineInfo(VK_TRUE, VK_TRUE);
    const auto vertexInputState = ShaderManager::GetVertexInputStateInfo();

	std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
	for (const auto& shader : material->GetShaders())
	{
		shaderStages.push_back(shader->GetStageInfo());
	}

    const VkGraphicsPipelineCreateInfo pipelineInfo
    {
        .sType =  VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext = &pipelineRenderingCreateInfo,
        .stageCount = static_cast<uint32_t>(shaderStages.size()),
        .pStages = shaderStages.data(),
        .pVertexInputState = &vertexInputState,
        .pInputAssemblyState = &inputAssemblyState,
        .pViewportState = &viewportState,
        .pRasterizationState = &rasterizer,
        .pMultisampleState = &multisampling,
        .pDepthStencilState = &depthStencilState,
        .pColorBlendState = &colorBlending,
        .pDynamicState = &dynamicState,
        .layout = graphicsPipeline.m_PipelineLayout,
        .renderPass = VK_NULL_HANDLE,
        .subpass = 0,
        .basePipelineHandle = VK_NULL_HANDLE,
        .basePipelineIndex = -1
    };

    LogInfo("Creating Pipeline For: " + material->GetMaterialName());
    VulkanCheck(vkCreateGraphicsPipelines(vulkanContext->device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline.m_GraphicsPipeline), "Failed to create graphics pipeline!")

    LogInfo("Pipeline Created For: " + material->GetMaterialName());
}