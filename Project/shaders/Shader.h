#pragma once
#include "vulkanbase/VulkanBase.h"

struct Vertex;
class Shader final
{
public:
	Shader() = default;
	~Shader() = default;
	Shader(const Shader&) = delete;
	Shader& operator=(const Shader&) = delete;
	Shader(Shader&&) = delete;
	Shader& operator=(Shader&&) = delete;

	static VkPipelineShaderStageCreateInfo CreateShaderInfo(const VkDevice& device, VkShaderStageFlagBits shaderStage, const std::string& fileName);
	static VkPipelineVertexInputStateCreateInfo CreateVertexInputStateInfo();
	static VkPipelineInputAssemblyStateCreateInfo CreateInputAssemblyStateInfo();

private:
	static VkShaderModule CreateShaderModule(const VkDevice& device, const std::vector<char>& code);

	//TODO: the lifetime of these variables are too long
	inline static VkVertexInputBindingDescription m_VertexInputBindingDescription = Vertex::GetBindingDescription();
	inline static std::array<VkVertexInputAttributeDescription, 3> m_VertexInputAttributeDescription = Vertex::GetAttributeDescriptions();
};