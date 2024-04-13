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
	explicit Material(VulkanContext* vulkanContext)
	: m_pContext(vulkanContext)
	{
		m_Shaders.reserve(2);
		m_pGraphicsPipeline = std::make_unique<GraphicsPipeline>();
	}
	~Material() = default;

	Material(const Material&) = delete;
	Material& operator=(const Material&) = delete;
	Material(Material&&) = delete;
	Material& operator=(Material&&) = delete;

	void CleanUp() const;

	void Bind(VkCommandBuffer commandBuffer, const glm::mat4x4& pushConstantMatrix);
	Shader* AddShader(const std::string& shaderPath, ShaderType shaderType);

	std::vector<Shader*> GetShaders() const
	{
		return m_Shaders;
	}


	void CreatePipeline();

	VkDescriptorSet& GetDescriptorSet()
	{
		return m_DescriptorSet;
	}


private:
	std::unique_ptr<GraphicsPipeline> m_pGraphicsPipeline;
	std::vector<Shader*> m_Shaders;


	VkDescriptorSet m_DescriptorSet{};
	VulkanContext* m_pContext;
};