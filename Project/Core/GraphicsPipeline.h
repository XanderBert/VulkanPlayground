#pragma once
#include <array>
#include <cassert>
#include <stdexcept>
#include <vector>
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>
#include <vulkan/vulkan.h>

#include "RenderPass.h"

class VulkanContext;
class Shader;
class GraphicsPipeline final
{
public:
	GraphicsPipeline() = default;
	~GraphicsPipeline() = default;


	GraphicsPipeline(const GraphicsPipeline&) = delete;
	GraphicsPipeline& operator=(const GraphicsPipeline&) = delete;
	GraphicsPipeline(GraphicsPipeline&&) = delete;
	GraphicsPipeline& operator=(GraphicsPipeline&&) = delete;

	void CreatePipeline(const VulkanContext* vulkanContext, const std::vector<VkPipelineShaderStageCreateInfo>& shaders, VkDescriptorSetLayout descriptorSetLayout);

	void BindPushConstant(const VkCommandBuffer commandBuffer, const glm::mat4x4& matrix) const;

	void BindPipeline(const VkCommandBuffer& commandBuffer) const;

	void Cleanup(const VkDevice& device) const
	{
		vkDestroyPipeline(device, m_GraphicsPipeline, nullptr);
		vkDestroyPipelineLayout(device, m_PipelineLayout, nullptr);
	}

	const VkPipeline& GetPipeline() const
	{
		return m_GraphicsPipeline;
	}

	const VkPipelineLayout& GetPipelineLayout() const
	{
		return m_PipelineLayout;
	}

private:
	friend class GraphicsPipelineBuilder;


	void SetShaders(const std::vector<VkPipelineShaderStageCreateInfo>& shaders)
	{
		m_ActiveShaders = shaders;
	}
	
	bool HasShaders() const
	{
		return !m_ActiveShaders.empty();
	}


	std::vector<VkPipelineShaderStageCreateInfo> m_ActiveShaders{};
	VkPipelineLayout m_PipelineLayout{};
	VkPipeline m_GraphicsPipeline{};
};




class GraphicsPipelineBuilder final
{
public:
	GraphicsPipelineBuilder() = default;
	~GraphicsPipelineBuilder() = default;
	GraphicsPipelineBuilder(const GraphicsPipelineBuilder&) = delete;
	GraphicsPipelineBuilder& operator=(const GraphicsPipelineBuilder&) = delete;
	GraphicsPipelineBuilder(GraphicsPipelineBuilder&&) = delete;
	GraphicsPipelineBuilder& operator=(GraphicsPipelineBuilder&&) = delete;

	static void CreatePipeline(GraphicsPipeline& graphicsPipeline, const VulkanContext* vulkanContext, VkDescriptorSetLayout descriptorSetLayout);


private:
	static VkPipelineViewportStateCreateInfo CreateViewportState()
	{
		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.scissorCount = 1;

		return viewportState;
	}

	static VkPipelineRasterizationStateCreateInfo CreateRasterizer()
	{
		VkPipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;

		return rasterizer;
	}

	static VkPipelineMultisampleStateCreateInfo CreateMultisampling()
	{
		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		return multisampling;
	}

	static VkPipelineColorBlendStateCreateInfo CreateColorBlending(VkPipelineColorBlendAttachmentState colorBlendAttachment)
	{
		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f;
		colorBlending.blendConstants[1] = 0.0f;
		colorBlending.blendConstants[2] = 0.0f;
		colorBlending.blendConstants[3] = 0.0f;

		return colorBlending;
	}

	static VkPipelineDynamicStateCreateInfo CreateDynamicState(const std::vector<VkDynamicState>& dynamicStates)
	{

		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
		dynamicState.pDynamicStates = dynamicStates.data();

		return dynamicState;
	}

	static void CreatePipelineLayout(const VkDevice& device, VkPipelineLayout& pipelineLayout, VkDescriptorSetLayout descriptorSetLayout);
};