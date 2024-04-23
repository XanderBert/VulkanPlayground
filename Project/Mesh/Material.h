#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <string>
#include <unordered_map>
#include <glm/glm.hpp>

#include <Core/Descriptor.h>
#include <Core/GraphicsPipeline.h>
#include <glm/gtc/type_ptr.hpp>

#include "Core/GlobalDescriptor.h"
#include "shaders/Logic/Shader.h"

enum class ShaderType;

namespace Descriptor
{
	class DescriptorLayoutCache;
	class DescriptorAllocator;
}


class Shader;
class VulkanContext;

class Material final
{
public:
	explicit Material(VulkanContext* vulkanContext, const std::string& materialName)
	: m_pContext(vulkanContext)
	, m_MaterialName(materialName)
	{
		m_Shaders.reserve(2);
		m_pGraphicsPipeline = std::make_unique<GraphicsPipeline>();

		m_pContext = vulkanContext;
	}
	~Material() = default;

	Material(const Material&) = delete;
	Material& operator=(const Material&) = delete;
	Material(Material&&) = delete;
	Material& operator=(Material&&) = delete;

	void OnImGui();

	void Bind(VkCommandBuffer commandBuffer, const glm::mat4x4& pushConstantMatrix);

	Shader* AddShader(const std::string& shaderPath, ShaderType shaderType);
	void ReloadShaders(Shader* shader);


	VkPipelineLayoutCreateInfo GetPipelineLayoutCreateInfo() const
	{
		//Push Constant in the Vertex Shader for model
		static VkPushConstantRange pushConstantRange{};
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(glm::mat4x4);
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		const VkDescriptorSetLayout layouts[] = { GlobalDescriptor::GetLayout(), m_MaterialDescriptorSetLayout };

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 2;
		pipelineLayoutInfo.pSetLayouts = layouts;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
		pipelineLayoutInfo.pushConstantRangeCount = 1;

		return pipelineLayoutInfo;
	}

	std::vector<Shader*> GetShaders() const
	{
		return m_Shaders;
	}

	VkDescriptorSet& GetDescriptorSet()
	{
		return m_MaterialDescriptorSet;
	}


	std::string GetMaterialName() const
	{
		return m_MaterialName;
	}

private:
	friend class MaterialManager;
	void CreatePipeline();
	void CleanUp() const;

	std::unique_ptr<GraphicsPipeline> m_pGraphicsPipeline;
	std::vector<Shader*> m_Shaders;

	VulkanContext* m_pContext;
	std::string m_MaterialName;


	DynamicBuffer m_MaterialUniformBuffer{};
	VkDescriptorSetLayout m_MaterialDescriptorSetLayout{};
	VkDescriptorSet m_MaterialDescriptorSet{};

	Descriptor::DescriptorWriter m_MaterialWriter{};
};