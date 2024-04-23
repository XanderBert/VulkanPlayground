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
	class DescriptorWriter;
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


//TODO: pad the dynamic buffer to 256 bytes
//TODO: return actual pointers to the data instead of the handle, Or make a handle struct
class DynamicBuffer final
{
public:
	DynamicBuffer() = default;
	~DynamicBuffer() = default;

	DynamicBuffer& operator=(const DynamicBuffer&) = delete;
	DynamicBuffer(const DynamicBuffer&) = delete;
	DynamicBuffer(DynamicBuffer&&) = delete;
	DynamicBuffer& operator=(DynamicBuffer&&) = delete;

	void Init(VulkanContext* vulkanContext);
	void ProperBind(int bindingNumber, const VkDescriptorSet& descriptorSet, Descriptor::DescriptorWriter& descriptorWriter, VulkanContext* vulkanContext);


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

	bool m_HasUniformBuffer = false;

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