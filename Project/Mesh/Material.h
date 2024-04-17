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

	std::vector<Shader*> GetShaders() const
	{
		return m_Shaders;
	}
	VkDescriptorSet& GetDescriptorSet()
	{
		return m_DescriptorSet;
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

	VkDescriptorSet m_DescriptorSet{};
	VulkanContext* m_pContext;

	std::string m_MaterialName;
};