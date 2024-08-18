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

    //TODO: I Need a proper way to pass a ARRAY of light structurs to the GPU.
    const std::vector<Light*> Lights = LightManager::GetLights();
    Light* light = Lights[0];

    lightPositionHandle = m_GlobalBuffer.AddVariable(glm::vec4(light->GetPosition()[0], light->GetPosition()[1], light->GetPosition()[2], 1.0f));
    lightColorHandle = m_GlobalBuffer.AddVariable(glm::vec4(light->GetColor()[0], light->GetColor()[1], light->GetColor()[2], 1.0f));

	m_GlobalBuffer.Init(vulkanContext);

	Descriptor::DescriptorBuilder builder{};
	builder.AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
	m_GlobalDescriptorSetLayout = builder.Build(vulkanContext->device, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
}



void GlobalDescriptor::Bind(VulkanContext* vulkanContext, const VkCommandBuffer commandBuffer, const VkPipelineLayout& pipelineLayout)
{
    LogAssert(m_GlobalDescriptorSetLayout != VK_NULL_HANDLE, "GlobalDescriptorSetLayout is not initialized", true);
    LogAssert(pipelineLayout != VK_NULL_HANDLE, "PipelineLayout is not initialized", true);

	m_GlobalBuffer.UpdateVariable(0, Camera::GetViewProjectionMatrix());
	m_GlobalBuffer.UpdateVariable(cameraHandle, glm::vec4(Camera::GetPosition(), 1.0f));
	m_GlobalBuffer.UpdateVariable(cameraPlaneHandle, glm::vec4(Camera::GetNearPlane(), Camera::GetFarPlane(), 0.0f, 0.0f));



    //TODO: I Need a proper way to pass a ARRAY of light structurs to the GPU.
    const std::vector<Light*> Lights = LightManager::GetLights();
    Light* light = Lights[0];
    m_GlobalBuffer.UpdateVariable(lightPositionHandle, glm::vec4(light->GetPosition()[0], light->GetPosition()[1], light->GetPosition()[2], 1.0f));
    m_GlobalBuffer.UpdateVariable(lightColorHandle, glm::vec4(light->GetColor()[0], light->GetColor()[1], light->GetColor()[2], 1.0f));



    m_GlobalDescriptorSet = Descriptor::DescriptorManager::Allocate(vulkanContext->device, m_GlobalDescriptorSetLayout, 0);
    m_Writer.Cleanup();


    m_GlobalBuffer.ProperBind(0, m_Writer);
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
