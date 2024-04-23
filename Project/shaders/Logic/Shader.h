#pragma once
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <vulkan/vulkan.h>
#include "Mesh/Vertex.h"

class Material;
class VulkanContext;

enum class ShaderType
{
	VertexShader = VK_SHADER_STAGE_VERTEX_BIT,
	FragmentShader = VK_SHADER_STAGE_FRAGMENT_BIT,
	GeometryShader = VK_SHADER_STAGE_GEOMETRY_BIT,
};

class Shader final
{
public:
	Shader(const VkPipelineShaderStageCreateInfo& shaderInfo, Material* material, const std::string& filename);
		
	~Shader() = default;
	Shader(const Shader&) = delete;
	Shader& operator=(const Shader&) = delete;
	Shader(Shader&&) = delete;
	Shader& operator=(Shader&&) = delete;

	VkPipelineShaderStageCreateInfo GetStageInfo() const;

	void OnImGui(const std::string& materialName);

	std::string GetFileName() const;
private:
	friend class ShaderManager;

	void AddMaterial(Material* material);
	void Cleanup(VkDevice device) const;
	void CleanupModule(VkDevice device) const;

	VkPipelineShaderStageCreateInfo m_ShaderInfo{};
	std::vector<Material*>  m_pMaterials;

	std::string m_FileName;
};

class ShaderManager final
{
public:
	ShaderManager() = default;
	~ShaderManager() = default;
	ShaderManager(const ShaderManager&) = delete;
	ShaderManager& operator=(const ShaderManager&) = delete;
	ShaderManager(ShaderManager&&) = delete;
	ShaderManager& operator=(ShaderManager&&) = delete;

	static void ReloadShader(VulkanContext* vulkanContext, const std::string& fileName, ShaderType shaderType);

	static Shader* CreateShader(VulkanContext* vulkanContext, const std::string& fileName, ShaderType shaderType, Material* material);

	static void Cleanup(VkDevice device);

	static VkPipelineVertexInputStateCreateInfo GetVertexInputStateInfo();
	static VkPipelineInputAssemblyStateCreateInfo GetInputAssemblyStateInfo();
private:
	class ShaderBuilder final
	{
	public:
		ShaderBuilder() = default;
		~ShaderBuilder() = default;
		ShaderBuilder(const ShaderBuilder&) = delete;
		ShaderBuilder& operator=(const ShaderBuilder&) = delete;
		ShaderBuilder(ShaderBuilder&&) = delete;
		ShaderBuilder& operator=(ShaderBuilder&&) = delete;

		static VkPipelineShaderStageCreateInfo CreateShaderInfo(const VkDevice& device, VkShaderStageFlagBits shaderStage, const std::string& fileName);


	private:
		static VkShaderModule CreateShaderModule(const VkDevice& device, const std::vector<char>& code);
	};

	inline static std::map<std::string, std::unique_ptr<Shader>> m_ShaderInfo;

	//TODO: the lifetime of these variables are too long
	inline static VkVertexInputBindingDescription m_VertexInputBindingDescription = Vertex::GetBindingDescription();
	inline static std::array<VkVertexInputAttributeDescription, 3> m_VertexInputAttributeDescription = Vertex::GetAttributeDescriptions();
};