#pragma once
#include "vulkanbase/VulkanBase.h"

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

};