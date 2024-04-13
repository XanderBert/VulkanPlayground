#include "Shader.h"
#include "Mesh/Material.h"
#include "vulkanbase/VulkanTypes.h"
#include <vector>



//---------------------------------------------------------------
//-------------------------DynamicBuffer-------------------------
//---------------------------------------------------------------
void DynamicBuffer::BindBuffer(VulkanContext* vulkanContext, int binding, VkShaderStageFlags shaderType, VkDescriptorSet& descriptorSet, VkDescriptorSetLayout& shaderDescriptorSetLayout)
{
	//Setup the uniform buffer
	const VkDeviceSize bufferSize = m_pDynamicBufferObject.GetSize();
	Core::Buffer::CreateBuffer(vulkanContext, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_UniformBuffer, m_UniformBuffersMemory);
	vkMapMemory(vulkanContext->device, m_UniformBuffersMemory, 0, bufferSize, 0, &m_UniformBuffersMapped);

	VkDescriptorBufferInfo bufferInfo{};
	bufferInfo.buffer = m_UniformBuffer;
	bufferInfo.offset = 0;
	bufferInfo.range = m_pDynamicBufferObject.GetSize();


	Descriptor::DescriptorManager::GetBuilder().BindBuffer(binding, &bufferInfo, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, shaderType).Build(descriptorSet, shaderDescriptorSetLayout );
}


void DynamicBuffer::Bind(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, VkDescriptorSet descriptorSet) const
{
	//Bind the descriptor set for the uniform buffer
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);

	//Copy the dynamic buffer to the mapped buffer
	memcpy(m_UniformBuffersMapped, m_pDynamicBufferObject.GetData(), m_pDynamicBufferObject.GetSize());
}

void DynamicBuffer::Cleanup(VkDevice device) const
{
	vkDestroyBuffer(device, m_UniformBuffer, nullptr);
	vkFreeMemory(device, m_UniformBuffersMemory, nullptr);
}

uint16_t DynamicBuffer::AddMatrix(const glm::mat4& matrix)
{
	return m_pDynamicBufferObject.AddMatrix(matrix);
}

void DynamicBuffer::UpdateMatrix(uint16_t handle, glm::mat4& matrix)
{
	m_pDynamicBufferObject.UpdateMatrix(handle, matrix);
}


//---------------------------------------------------------------------
//-------------------------DynamicBufferObject-------------------------
//---------------------------------------------------------------------

uint16_t DynamicBuffer::DynamicBufferObject::AddMatrix(const glm::mat4& matrix)
{
	constexpr size_t size = sizeof(glm::mat4) / sizeof(float);

	const float* ptr = glm::value_ptr(matrix);

	data.insert(data.end(), ptr, ptr + size);

	return data.size() - size;
}

void DynamicBuffer::DynamicBufferObject::UpdateMatrix(uint16_t handle, glm::mat4& matrix)
{
	constexpr size_t size = sizeof(glm::mat4) / sizeof(float);

	LogAssert(handle + size <= data.size(), "Matrix Handle out of bounds", true)

	std::copy_n(value_ptr(matrix), size, data.begin() + handle);
}

const float* DynamicBuffer::DynamicBufferObject::GetData() const
{
	return data.data();
}

size_t DynamicBuffer::DynamicBufferObject::GetSize() const
{
	return data.size() * sizeof(float);
}




//--------------------------------------------------------
//-------------------------Shader-------------------------
//--------------------------------------------------------

void Shader::AddMaterial(Material* material)
{
	m_pMaterials.push_back(material);
}

void Shader::AddUniformBuffer(VulkanContext* vulkanContext, int binding, Material* material)
{
	//Check if the material is member of
	const auto it = std::find(m_pMaterials.begin(), m_pMaterials.end(), material);
	LogAssert(it != m_pMaterials.end(), "The passed material does not use this shader!", true)

	if(m_HasUniformBuffer)
	{
		//Build the uniform buffer for this material
		m_DynamicBuffer.BindBuffer(vulkanContext, binding, m_ShaderInfo.stage, material->GetDescriptorSet(), m_DescriptorSetLayout);
	}
}

VkDescriptorSetLayout& Shader::GetDescriptorSetLayout()
{
	return m_DescriptorSetLayout;
}

void Shader::Cleanup(VkDevice device) const
{
	if (m_HasUniformBuffer)
		m_DynamicBuffer.Cleanup(device);

	vkDestroyShaderModule(device, m_ShaderInfo.module, nullptr);
}

Shader::Shader(const VkPipelineShaderStageCreateInfo& shaderInfo, Material* material)
	: m_ShaderInfo(shaderInfo)
	, m_pMaterials({ material })
{

}

uint16_t Shader::AddMatrix(const glm::mat4& matrix, VulkanContext* vulkanContext, int binding)
{
	//if (!m_HasUniformBuffer)
	//{
	//	LogAssert(vulkanContext, "When adding your first variable you need to pass the Context", true)
	//	if (!vulkanContext) return 0;


	//	const uint16_t handle = m_DynamicBuffer.AddMatrix(matrix);

	//	AddUniformBuffer(vulkanContext, binding);

	//	return handle;
	//}
	m_HasUniformBuffer = true;
	return m_DynamicBuffer.AddMatrix(matrix);
}

void Shader::Bind(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, Material* material) const
{
	LogAssert(material, "Material is nullptr", true)

	if (m_HasUniformBuffer)
		m_DynamicBuffer.Bind(commandBuffer, pipelineLayout, material->GetDescriptorSet());

}

VkPipelineShaderStageCreateInfo Shader::GetStageInfo() const
{
	return m_ShaderInfo;
}




//---------------------------------------------------------------
//-------------------------ShaderManager-------------------------
//---------------------------------------------------------------

VkPipelineShaderStageCreateInfo ShaderManager::ShaderBuilder::CreateShaderInfo(const VkDevice& device, VkShaderStageFlagBits shaderStage, const std::string& fileName)
{
	const std::string fileLocation = "shaders/" + fileName + ".spv";
	const std::vector<char> shaderCode = tools::readFile(fileLocation);

	LogAssert(!shaderCode.empty(), "Failed find the shader at the specified path", true)


	VkPipelineShaderStageCreateInfo shaderStageInfo{};
	shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStageInfo.stage = shaderStage;
	shaderStageInfo.module = CreateShaderModule(device, shaderCode);
	shaderStageInfo.pName = "main";

	return shaderStageInfo;
}

VkShaderModule ShaderManager::ShaderBuilder::CreateShaderModule(const VkDevice& device, const std::vector<char>& code)
{
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create shader module!");
	}

	return shaderModule;
}

Shader* ShaderManager::CreateShader(VulkanContext* vulkanContext, const std::string& fileName, ShaderType shaderType, Material* material)
{
	//Check if the shader already exists
	const auto it = m_ShaderInfo.find(fileName);
	
	if (it != m_ShaderInfo.end())
	{
		Shader* shader = it->second.get();
		shader->AddMaterial(material);
		return shader;
	}

	VkPipelineShaderStageCreateInfo shaderInfo = ShaderBuilder::CreateShaderInfo(vulkanContext->device, static_cast<VkShaderStageFlagBits>(shaderType), fileName);


	std::unique_ptr<Shader> shaderPtr = std::make_unique<Shader>(shaderInfo, material);
	Shader* shader = shaderPtr.get();

	m_ShaderInfo.insert(std::make_pair(fileName, std::move(shaderPtr)));

	return shader;
}

void ShaderManager::Cleanup(VkDevice device)
{
	for (const auto& shaderInfo : m_ShaderInfo)
	{
		shaderInfo.second->Cleanup(device);
	}

	m_ShaderInfo.clear();
}

VkPipelineVertexInputStateCreateInfo ShaderManager::GetVertexInputStateInfo()
{
	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &m_VertexInputBindingDescription;
	vertexInputInfo.vertexAttributeDescriptionCount = m_VertexInputAttributeDescription.size();
	vertexInputInfo.pVertexAttributeDescriptions = m_VertexInputAttributeDescription.data();

	return vertexInputInfo;
}

VkPipelineInputAssemblyStateCreateInfo ShaderManager::GetInputAssemblyStateInfo()
{
	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;
	return inputAssembly;
}