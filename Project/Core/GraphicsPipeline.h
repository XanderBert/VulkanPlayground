#pragma once
#include <vector>
#include <glm/mat4x4.hpp>
#include <vulkan/vulkan.h>

#include "Mesh/Material.h"

class Material;
class VulkanContext;
class Shader;

enum class PipelineType
{
    Graphics = VK_PIPELINE_BIND_POINT_GRAPHICS,
    Compute = VK_PIPELINE_BIND_POINT_COMPUTE,
    RayTracing = VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR
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

	static void CreatePipeline(const VulkanContext* vulkanContext, Material* material);
	static void CreatePipelineLayout(VkPipelineLayout& pipelineLayout, const VulkanContext* vulkanContext);
};