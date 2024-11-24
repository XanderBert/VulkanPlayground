#pragma once
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <vector>
#include <vulkan/vulkan.h>

#include "Core/DepthResource.h"
#include "Core/DescriptorSet.h"
#include "Core/GraphicsPipeline.h"

//Create a renderpass class

//This holds information about
//attachments
//pipeline to use (Global or Another one)
//


//Some render passes only "render" one material
//Other ones will go over each mesh to render

//Do i make a pure virtual renderpass?

//Do i create a different one for compute passes?


struct RenderPass
{
	RenderPass() = default;
	virtual ~RenderPass() = default;

	RenderPass(const RenderPass&) = delete;
	RenderPass(RenderPass&&) = delete;
	RenderPass& operator=(const RenderPass&) = delete;
	RenderPass& operator=(RenderPass&&) = delete;

	virtual	void Render(VkCommandBuffer commandBuffer) = 0;
};


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

	//void Bind()

    void Bind(VkCommandBuffer commandBuffer) const;
	//void BindPushConstant(VkCommandBuffer commandBuffer, const glm::mat4x4& pushConstantMatrix) const;

    //Checks if a shader with the same type already exists,
    //if so it removes it and adds the new one
    Shader* SetShader(const std::string& shaderPath, ShaderType shaderType);

    //Adds a shader of the specified type without checking if one is already present of the same type
    Shader* AddShader(const std::string& shaderPath, ShaderType shaderType);

    [[nodiscard]] const std::vector<Shader*>& GetShaders() const;
    [[nodiscard]] std::string GetMaterialName() const;
    [[nodiscard]] VkCullModeFlags GetCullModeBit() const;
    void SetCullMode(VkCullModeFlags cullMode);

	//TODO Clean this up -> Render graphs
    [[nodiscard]] bool GetDepthOnly() const;
    [[nodiscard]] bool IsCompute() const;
	[[nodiscard]] bool IsSSAO() const;
	[[nodiscard]] bool IsComposite() const;

    void SetDepthOnly(bool depthOnly);
	void SetSSAOPass(bool isSSAO);
	void SetIsComposite(bool isComposite);

private:
	friend class MaterialManager;
	friend class GraphicsPipelineBuilder;
	friend class ShaderManager;

	void CreatePipeline();
	void CleanupPipeline() const;

	VkPipeline m_GraphicsPipeline{};
	PipelineType m_PipelineType = PipelineType::Graphics;

	std::vector<Shader*> m_Shaders;
	VulkanContext* m_pContext;
	std::string m_MaterialName;

    VkCullModeFlags m_CullMode = VK_CULL_MODE_BACK_BIT;

	//TODO: Cleanup -> Render graphs
    bool m_IsDepthOnly = false;
	bool m_IsSSAO = false;
	bool m_IsComposite = false;


};