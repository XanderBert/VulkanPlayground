#pragma once
#include <vulkan/vulkan.h>
#include <utility>
#include <vector>
#include <memory>
#include <string>
#include <glm/glm.hpp>

#include <Core/Descriptor.h>
#include <Core/GraphicsPipeline.h>
#include <glm/gtc/type_ptr.hpp>

#include "Core/DescriptorSet.h"
#include "Core/DynamicUniformBuffer.h"
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
	explicit Material(VulkanContext *vulkanContext, std::string materialName);
    ~Material() = default;

	Material(const Material&) = delete;
	Material& operator=(const Material&) = delete;
	Material(Material&&) = delete;
	Material& operator=(Material&&) = delete;

	void OnImGui() const;

    void Bind(VkCommandBuffer commandBuffer, const glm::mat4x4& pushConstantMatrix);

    Shader* AddShader(const std::string& shaderPath, ShaderType shaderType);
	void ReloadShaders(Shader* shader);
    std::vector<Shader*> GetShaders() const;

    const VkPipelineLayout& GetPipelineLayout() const;

    VkPipelineLayoutCreateInfo GetPipelineLayoutCreateInfo();

    std::string GetMaterialName() const
	{
		return m_MaterialName;
	}

    DescriptorSet* GetDescriptorSet()
    {
        return &m_DescriptorSet;
    }

private:
	friend class MaterialManager;
	void CreatePipeline();
	void CleanUp();

	std::unique_ptr<GraphicsPipeline> m_pGraphicsPipeline;
	std::vector<Shader*> m_Shaders;

	VulkanContext* m_pContext;
	std::string m_MaterialName;

    DescriptorSet m_DescriptorSet{};
    std::vector<VkDescriptorSetLayout> m_SetLayouts{};
};