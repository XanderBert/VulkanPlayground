#pragma once
#include <vector>
#include <glm/mat4x4.hpp>
#include <vulkan/vulkan.h>

class Material;
class VulkanContext;
class Shader;

enum class PipelineType
{
    Graphics = VK_PIPELINE_BIND_POINT_GRAPHICS,
    Compute = VK_PIPELINE_BIND_POINT_COMPUTE,
    RayTracing = VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR
};

class GraphicsPipeline final
{
public:
	GraphicsPipeline() = default;
	~GraphicsPipeline() = default;


	GraphicsPipeline(const GraphicsPipeline&) = delete;
	GraphicsPipeline& operator=(const GraphicsPipeline&) = delete;
	GraphicsPipeline(GraphicsPipeline&&) = delete;
	GraphicsPipeline& operator=(GraphicsPipeline&&) = delete;

	void CreatePipeline(const VulkanContext* vulkanContext, Material* material);

	void BindPushConstant(const VkCommandBuffer commandBuffer, const glm::mat4x4& matrix) const;

	void BindPipeline(const VkCommandBuffer& commandBuffer, PipelineType pipeline = PipelineType::Graphics) const;

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

	static void CreatePipeline(GraphicsPipeline& graphicsPipeline, const VulkanContext* vulkanContext, Material* material);
};