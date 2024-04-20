#include "GlobalDescriptor.h"

#include "Descriptor.h"
#include "Camera/Camera.h"
#include "Shaders/Logic/Shader.h"


void GlobalDescriptor::Init(VulkanContext* vulkanContext)
{
	m_Texture = new Texture("texture.jpg", vulkanContext);


	m_GlobalBuffer.AddVariable(Camera::GetViewProjectionMatrix());
	cameraHandle = m_GlobalBuffer.AddVariable(glm::vec4(Camera::GetPosition(), 1.0f));


	m_Texture->BindImage(1);
	Descriptor::DescriptorManager::GetBuilder().Build(m_DescriptorSet, m_DescriptorSetLayout);
	//

	m_GlobalBuffer.BindBuffer(vulkanContext, 0, VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT, m_GlobalDescriptorSet, m_GlobalDescriptorSetLayout);
}

void GlobalDescriptor::Bind(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout)
{
	m_GlobalBuffer.UpdateVariable(0, Camera::Camera::GetViewProjectionMatrix());
	m_GlobalBuffer.UpdateVariable(cameraHandle, glm::vec4(Camera::GetPosition(), 1.0f));
	m_GlobalBuffer.Bind(commandBuffer, pipelineLayout, m_GlobalDescriptorSet);
}

VkDescriptorSetLayout& GlobalDescriptor::GetLayout()
{
	return m_GlobalDescriptorSetLayout;
}

void GlobalDescriptor::Cleanup(VkDevice device) const
{
	m_Texture->Cleanup(device);
	m_GlobalBuffer.Cleanup(device);
}
