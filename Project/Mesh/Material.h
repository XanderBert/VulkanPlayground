#pragma once
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <vector>
#include <vulkan/vulkan.h>
#include "Core/DescriptorSet.h"
#include "Core/GraphicsPipeline.h"


enum class ShaderType;
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


	void OnImGui();

    void Bind(VkCommandBuffer commandBuffer, const glm::mat4x4& pushConstantMatrix);

    //Checks if a shader with the same type already exists,
    //if so it removes it and adds the new one
    Shader* SetShader(const std::string& shaderPath, ShaderType shaderType);

    //Adds a shader of the specified type without checking if one is already present of the same type
    Shader* AddShader(const std::string& shaderPath, ShaderType shaderType);

    void CreatePipeline();

    [[nodiscard]] std::vector<Shader*> GetShaders() const;
    [[nodiscard]] const VkPipelineLayout& GetPipelineLayout() const;
    [[nodiscard]] VkPipelineLayoutCreateInfo GetPipelineLayoutCreateInfo();
    [[nodiscard]] std::string GetMaterialName() const;

    [[nodiscard]] DescriptorSet* GetDescriptorSet();

    [[nodiscard]] VkCullModeFlags GetCullModeBit() const;
    void SetCullMode(VkCullModeFlags cullMode);

    [[nodiscard]] bool GetDepthOnly() const;
    [[nodiscard]] bool IsCompute() const;
    void SetDepthOnly(bool depthOnly);
private:
	friend class MaterialManager;

	void CleanUp();

	std::unique_ptr<GraphicsPipeline> m_pGraphicsPipeline;
	std::vector<Shader*> m_Shaders;

	VulkanContext* m_pContext;
	std::string m_MaterialName;

    DescriptorSet m_DescriptorSet{};
    std::vector<VkDescriptorSetLayout> m_SetLayouts{};


    VkCullModeFlags m_CullMode = VK_CULL_MODE_BACK_BIT;

    bool m_IsDepthOnly = false;

    PipelineType m_PipelineType = PipelineType::Graphics;
};