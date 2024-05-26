#include "GlobalDescriptor.h"

#include "Descriptor.h"
#include "Camera/Camera.h"
#include "Shaders/Logic/Shader.h"
#include "DepthResource.h"

void GlobalDescriptor::Init(VulkanContext* vulkanContext)
{
   // m_Light = Light(vulkanContext);

	m_GlobalBuffer.AddVariable(Camera::GetViewProjectionMatrix());
	cameraHandle = m_GlobalBuffer.AddVariable(glm::vec4(Camera::GetPosition(), 1.0f));
    cameraPlaneHandle = m_GlobalBuffer.AddVariable(glm::vec4(Camera::GetNearPlane(), Camera::GetFarPlane(), 0.0f, 0.0f));

    // lightPositionHandle = m_GlobalBuffer.AddVariable(glm::vec4(m_Light.GetPosition()[0], m_Light.GetPosition()[1], m_Light.GetPosition()[2], 1.0f));
    // lightDirectionHandle = m_GlobalBuffer.AddVariable(glm::vec4(m_Light.GetDirection()[0], m_Light.GetDirection()[1], m_Light.GetDirection()[2], 1.0f));
    // lightColorHandle = m_GlobalBuffer.AddVariable(glm::vec4(m_Light.GetColor()[0], m_Light.GetColor()[1], m_Light.GetColor()[2], 1.0f));


	m_GlobalBuffer.Init(vulkanContext);






	Descriptor::DescriptorBuilder builder{};
	builder.AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    //builder.AddBinding(1,VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
	m_GlobalDescriptorSetLayout = builder.Build(vulkanContext->device, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
}



void GlobalDescriptor::Bind(VulkanContext* vulkanContext, const VkCommandBuffer commandBuffer, const VkPipelineLayout& pipelineLayout)
{
	m_GlobalBuffer.UpdateVariable(0, Camera::GetViewProjectionMatrix());
	m_GlobalBuffer.UpdateVariable(cameraHandle, glm::vec4(Camera::GetPosition(), 1.0f));
	m_GlobalBuffer.UpdateVariable(cameraPlaneHandle, glm::vec4(Camera::GetNearPlane(), Camera::GetFarPlane(), 0.0f, 0.0f));

    // m_GlobalBuffer.UpdateVariable(lightPositionHandle, glm::vec4(m_Light.GetPosition()[0], m_Light.GetPosition()[1], m_Light.GetPosition()[2], 1.0f));
    // m_GlobalBuffer.UpdateVariable(lightDirectionHandle, glm::vec4(m_Light.GetDirection()[0], m_Light.GetDirection()[1], m_Light.GetDirection()[2], 1.0f));
    // m_GlobalBuffer.UpdateVariable(lightColorHandle, glm::vec4(m_Light.GetColor()[0], m_Light.GetColor()[1], m_Light.GetColor()[2], 1.0f));


	m_GlobalDescriptorSet = Descriptor::DescriptorManager::Allocate(vulkanContext->device, m_GlobalDescriptorSetLayout, 0);
    m_Writer.Cleanup();


    m_GlobalBuffer.ProperBind(0, m_Writer);
    //m_Writer.WriteImage(1, m_Light.GetShadowMapView(), m_Light.GetShadowMapSampler(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    m_Writer.UpdateSet(vulkanContext->device, m_GlobalDescriptorSet);

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &m_GlobalDescriptorSet, 0, nullptr);
}

VkDescriptorSetLayout& GlobalDescriptor::GetLayout()
{
	return m_GlobalDescriptorSetLayout;
}


void GlobalDescriptor::Cleanup(VkDevice device)
{
	vkDestroyDescriptorSetLayout(device, m_GlobalDescriptorSetLayout, nullptr);
	m_GlobalBuffer.Cleanup(device);
}
