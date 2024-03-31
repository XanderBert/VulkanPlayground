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

namespace Descriptor
{
	class DescriptorLayoutCache;
	class DescriptorAllocator;
}


class Shader;
class VulkanContext;

enum class ShaderType
{
	VertexShader = VK_SHADER_STAGE_VERTEX_BIT,
	FragmentShader = VK_SHADER_STAGE_FRAGMENT_BIT,
	GeometryShader = VK_SHADER_STAGE_GEOMETRY_BIT,	
};




class DynamicBufferObject
{
public:

	uint16_t AddFloat3(glm::vec3 value)
	{
		data.push_back(value.x);
		data.push_back(value.y);
		data.push_back(value.z);
		data.push_back(0.0f); // Pad to vec4

		return data.size() - 4;
	}

	// Add a glm::mat4 value
	uint16_t AddMatrix(const glm::mat4& matrix)
	{
		const float* ptr = glm::value_ptr(matrix);
		data.insert(data.end(), ptr, ptr + 16);


		return data.size() - 16;
	}

	void UpdateMatrix(uint16_t handle, glm::mat4& matrix)
	{
		LogAssert(handle + 16 <= data.size(), "Matrix Handle out of bounds", true);

		float* ptr = glm::value_ptr(matrix);
		std::copy_n(ptr, 16, data.begin() + handle);
	}

	// Get pointer to raw data
	const float* GetData() const
	{
		return data.data();
	} 

	// Get size of data in bytes
	size_t GetSize() const
	{
		return data.size() * sizeof(float);
	}




private:
	std::vector<float> data; // Store all values as vec4 for memory alignment
};



class Material final
{
public:
	explicit Material(VulkanContext* vulkanContext)
		: m_UniformBuffer(nullptr)
		, m_UniformBuffersMemory(nullptr)
		, m_UniformBuffersMapped(nullptr),
		  m_pContext(vulkanContext)
	{
		m_Shaders.reserve(2);
		m_pGraphicsPipeline = std::make_unique<GraphicsPipeline>();

		m_pDescriptorAllocator = std::make_unique<Descriptor::DescriptorAllocator>();
		m_pDescriptorAllocator->Init(m_pContext->device);

		m_pDescriptorCache = std::make_unique<Descriptor::DescriptorLayoutCache>();
		m_pDescriptorCache->Init(m_pContext->device);

		m_DescriptorBuilder = Descriptor::DescriptorBuilder::Begin(m_pDescriptorCache.get(), m_pDescriptorAllocator.get());
	}
	~Material() = default;

	Material(const Material&) = delete;
	Material& operator=(const Material&) = delete;
	Material(Material&&) = delete;
	Material& operator=(Material&&) = delete;

	void CleanUp() const;

	void Bind(VkCommandBuffer commandBuffer) const;
	void AddShader(const std::string& shaderPath, ShaderType shaderType);

	//void AddShaderVariable(const glm::vec3& vector);
	uint16_t AddShaderVariable(const glm::mat4& matrix);
	void UpdateShaderVariable(uint16_t handle,  glm::mat4& matrix);


	void CreatePipeline();

private:
	DynamicBufferObject m_pDynamicBufferObject{};

	VkBuffer m_UniformBuffer;
	VkDeviceMemory m_UniformBuffersMemory;
	void* m_UniformBuffersMapped;


	std::unique_ptr<GraphicsPipeline> m_pGraphicsPipeline;
	std::vector<VkPipelineShaderStageCreateInfo> m_Shaders;
	
	
	VulkanContext* m_pContext;
	VkDescriptorSet m_DescriptorSet{};
	VkDescriptorSetLayout m_DescriptorSetLayout{};


	//TOOD Move this to a separate class
	std::unique_ptr<Descriptor::DescriptorAllocator> m_pDescriptorAllocator;
	std::unique_ptr<Descriptor::DescriptorLayoutCache> m_pDescriptorCache;
	Descriptor::DescriptorBuilder m_DescriptorBuilder;
};