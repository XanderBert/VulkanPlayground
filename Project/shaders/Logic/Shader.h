#pragma once
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <glm/mat4x4.hpp>
#include <vulkan/vulkan.h>

#include "Mesh/Vertex.h"

namespace Descriptor
{
	class DescriptorBuilder;
}

class Material;
class VulkanContext;

enum class ShaderType
{
	VertexShader = VK_SHADER_STAGE_VERTEX_BIT,
	FragmentShader = VK_SHADER_STAGE_FRAGMENT_BIT,
	GeometryShader = VK_SHADER_STAGE_GEOMETRY_BIT,
};

class DynamicBuffer final
{
public:
	DynamicBuffer() = default;
	~DynamicBuffer() = default;

	DynamicBuffer& operator=(const DynamicBuffer&) = delete;
	DynamicBuffer(const DynamicBuffer&) = delete;
	DynamicBuffer(DynamicBuffer&&) = delete;
	DynamicBuffer& operator=(DynamicBuffer&&) = delete;

	void BindBuffer(VulkanContext* vulkanContext, int binding, VkShaderStageFlags shaderType, VkDescriptorSet& descriptorSet, VkDescriptorSetLayout& shaderDescriptorSetLayout);
	void Bind(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, VkDescriptorSet descriptorSet) const;

	void Cleanup(VkDevice device) const;

	uint16_t AddVariable(const glm::vec4& value);
	uint16_t AddVariable(const glm::mat4& matrix);
	void UpdateVariable(uint16_t handle, const glm::mat4& matrix);
	void UpdateVariable(uint16_t handle, const glm::vec4& value);

private:
	const float* GetData() const;
	size_t GetSize() const;

	uint16_t Insert(const float* dataPtr, uint8_t size);
	void Update(uint16_t handle, const float* dataPtr, uint8_t size);
	std::vector<float> m_Data;
	

	VkBuffer m_UniformBuffer;
	VkDeviceMemory m_UniformBuffersMemory;
	void* m_UniformBuffersMapped;
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

	//uint16_t AddMatrix(const glm::mat4& matrix, VulkanContext* vulkanContext = nullptr, int binding = 0);
	void Bind(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, Material* material) const;

	VkPipelineShaderStageCreateInfo GetStageInfo() const;

	void AddUniformBuffer(VulkanContext* vulkanContext, int binding, Material* material);

	VkDescriptorSetLayout& GetDescriptorSetLayout();

	void OnImGui();

	std::string GetFileName() const;
private:
	friend class ShaderManager;

	void AddMaterial(Material* material);
	void Cleanup(VkDevice device) const;
	void CleanupModule(VkDevice device) const;

	VkPipelineShaderStageCreateInfo m_ShaderInfo{};
	std::vector<Material*>  m_pMaterials;

	bool m_HasUniformBuffer = false;

	DynamicBuffer m_DynamicBuffer{};

	//Todo:: A Shader will need a descriptor set for every frame in flight
	//you cannot update a set that has been used, so if you ever plan on changing them you need one per material per frame in flight
	VkDescriptorSetLayout m_DescriptorSetLayout{};

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