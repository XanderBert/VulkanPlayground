#include "vulkanbase/VulkanBase.h"
#include <Core/GraphicsPipeline.h>

#include "Core/RenderPass.h"
#include "shaders/Shader.h"

void VulkanBase::createFrameBuffers()
{
	swapChainFramebuffers.resize(swapChainImageViews.size());

	for (size_t i = 0; i < swapChainImageViews.size(); i++) 
	{
		const VkImageView attachments[] = 
		{
			swapChainImageViews[i]
		};

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = m_pGraphicsPipeline.GetRenderPass();
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = attachments;
		framebufferInfo.width = swapChainExtent.width;
		framebufferInfo.height = swapChainExtent.height;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(m_pContext->device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) 
		{
			throw std::runtime_error("failed to create framebuffer!");
		}
	}
}





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
