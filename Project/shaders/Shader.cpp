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
	const VkDeviceSize bufferSize = GetSize();
	Core::Buffer::CreateBuffer(vulkanContext, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_UniformBuffer, m_UniformBuffersMemory);
	vkMapMemory(vulkanContext->device, m_UniformBuffersMemory, 0, bufferSize, 0, &m_UniformBuffersMapped);

	VkDescriptorBufferInfo bufferInfo{};
	bufferInfo.buffer = m_UniformBuffer;
	bufferInfo.offset = 0;
	bufferInfo.range = GetSize();


	Descriptor::DescriptorManager::GetBuilder().BindBuffer(binding, &bufferInfo, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, shaderType).Build(descriptorSet, shaderDescriptorSetLayout );
}

void DynamicBuffer::Bind(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, VkDescriptorSet descriptorSet) const
{
	//Bind the descriptor set for the uniform buffer
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);

	//Copy the dynamic buffer to the mapped buffer
	memcpy(m_UniformBuffersMapped, GetData(), GetSize());
}

void DynamicBuffer::Cleanup(VkDevice device) const
{
	vkDestroyBuffer(device, m_UniformBuffer, nullptr);
	vkFreeMemory(device, m_UniformBuffersMemory, nullptr);
}

uint16_t DynamicBuffer::AddVariable(const glm::vec4& value)
{
	constexpr uint8_t size = sizeof(glm::vec4) / sizeof(float);
	return Insert(glm::value_ptr(value), size);
}

uint16_t DynamicBuffer::AddVariable(const glm::mat4& value)
{
	constexpr uint8_t size = sizeof(glm::mat4) / sizeof(float);
	return Insert(glm::value_ptr(value), size);
}

void DynamicBuffer::UpdateVariable(uint16_t handle, const glm::vec4& value)
{
	constexpr uint8_t size = sizeof(glm::vec4) / sizeof(float);
	Update(handle, glm::value_ptr(value), size);
}

void DynamicBuffer::UpdateVariable(uint16_t handle, const glm::mat4& value)
{
	constexpr uint8_t size = sizeof(glm::mat4) / sizeof(float);
	Update(handle, glm::value_ptr(value), size);
}

//---------------------------------------------------------------------
//-------------------------DynamicBufferObject-------------------------
//---------------------------------------------------------------------
const float* DynamicBuffer::GetData() const
{
	return m_Data.data();
}

size_t DynamicBuffer::GetSize() const
{
	return m_Data.size() * sizeof(float);
}

uint16_t DynamicBuffer::Insert(const float* dataPtr, uint8_t size)
{
	//Check if the size is a multiple of vec4
	LogAssert(size % 4 == 0, "Size is not a multiple of 4", true)

	m_Data.insert(m_Data.end(), dataPtr, dataPtr + size);

	return m_Data.size() - size;
}

void DynamicBuffer::Update(uint16_t handle, const float* dataPtr, uint8_t size)
{
	LogAssert(handle + size <= m_Data.size(), "Handle out of bounds", true)

	std::copy_n(dataPtr, size, m_Data.begin() + handle);
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