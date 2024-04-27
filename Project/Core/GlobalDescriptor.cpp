#include "GlobalDescriptor.h"

#include "Descriptor.h"
#include "Camera/Camera.h"
#include "Shaders/Logic/Shader.h"


void GlobalDescriptor::Init(VulkanContext* vulkanContext)
{
	m_GlobalBuffer.AddVariable(Camera::GetViewProjectionMatrix());
	cameraHandle = m_GlobalBuffer.AddVariable(glm::vec4(Camera::GetPosition(), 1.0f));
	m_GlobalBuffer.Init(vulkanContext);

	Descriptor::DescriptorBuilder builder{};
	builder.AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
	m_GlobalDescriptorSetLayout = builder.Build(vulkanContext->device, VK_SHADER_STAGE_VERTEX_BIT);
}



void GlobalDescriptor::Bind(VulkanContext* vulkanContext, const VkCommandBuffer commandBuffer, const VkPipelineLayout& pipelineLayout)
{
	m_GlobalBuffer.UpdateVariable(0, Camera::GetViewProjectionMatrix());
	m_GlobalBuffer.UpdateVariable(cameraHandle, glm::vec4(Camera::GetPosition(), 1.0f));

	m_GlobalDescriptorSet = Descriptor::DescriptorManager::Allocate(vulkanContext->device, m_GlobalDescriptorSetLayout, 0);

	m_GlobalBuffer.FullRebind(0, m_GlobalDescriptorSet, m_Writer, vulkanContext);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &GetDescriptorSet(), 0, nullptr);
}

VkDescriptorSetLayout& GlobalDescriptor::GetLayout()
{
	return m_GlobalDescriptorSetLayout;
}

VkDescriptorSet& GlobalDescriptor::GetDescriptorSet()
{
	return m_GlobalDescriptorSet;
}

void GlobalDescriptor::Cleanup(VkDevice device)
{
	vkDestroyDescriptorSetLayout(device, m_GlobalDescriptorSetLayout, nullptr);
	m_GlobalBuffer.Cleanup(device);
}
