#include "Core/GlobalDescriptor.h"

#include "DepthResource.h"
#include "Descriptor.h"
#include "Camera/Camera.h"
#include "shaders/Logic/Shader.h"

void GlobalDescriptor::Init(VulkanContext *vulkanContext)
{
	m_pVulkanContext = vulkanContext;
	GraphicsPipelineBuilder::CreatePipelineLayout(m_PipelineLayout, m_pVulkanContext);

	//TODO: I Need a proper way to pass a ARRAY of light structurs to the GPU.
	const std::vector<Light *> Lights = LightManager::GetLights();
	Light *light = Lights[0];

	auto* globalBuffer = m_DescriptorSet.AddBuffer(0, DescriptorType::UniformBuffer);

	viewProjectionHandle = globalBuffer->AddVariable(Camera::GetViewProjectionMatrix());
	cameraHandle = globalBuffer->AddVariable(glm::vec4(Camera::GetPosition(), 1.0f));
	cameraPlaneHandle = globalBuffer->AddVariable(glm::vec4(Camera::GetNearPlane(), Camera::GetFarPlane(), 0.0f, 0.0f));
	lightPositionHandle = globalBuffer->AddVariable(glm::vec4(light->GetPosition()[0], light->GetPosition()[1], light->GetPosition()[2], 1.0f));
	lightColorHandle = globalBuffer->AddVariable(glm::vec4(light->GetColor()[0], light->GetColor()[1], light->GetColor()[2], 1.0f));
	inverseProjectionHandle = globalBuffer->AddVariable(inverse(Camera::GetProjectionMatrix()));
	viewMatrixHandle = globalBuffer->AddVariable(Camera::GetViewMatrix());
}


void GlobalDescriptor::Bind(VkCommandBuffer commandBuffer)
{
	//auto* globalBuffer = m_DescriptorSet.GetBuffer(0);

	//TODO --> Move to Update:
	//LogAssert(GetLayout() != VK_NULL_HANDLE, "GlobalDescriptorSetLayout is not initialized", true);
	//globalBuffer->UpdateVariable(viewProjectionHandle, Camera::GetViewProjectionMatrix());
	//globalBuffer->UpdateVariable(viewProjectionHandle, Camera::GetViewProjectionMatrix());
	//globalBuffer->UpdateVariable(cameraHandle, glm::vec4(Camera::GetPosition(), 1.0f));
	//globalBuffer->UpdateVariable(cameraPlaneHandle, glm::vec4(Camera::GetNearPlane(), Camera::GetFarPlane(), 0.0f, 0.0f));
	//globalBuffer->UpdateVariable(inverseProjectionHandle, inverse(Camera::GetProjectionMatrix()));
	//globalBuffer->UpdateVariable(viewMatrixHandle, Camera::GetViewMatrix());

	//TODO: I Need a proper way to pass a ARRAY of light structurs to the GPU.
	//const std::vector<Light *> Lights = LightManager::GetLights();
	//Light *light = Lights[0];
	//globalBuffer->UpdateVariable(lightPositionHandle, glm::vec4(light->GetPosition()[0], light->GetPosition()[1], light->GetPosition()[2], 1.0f));
	//globalBuffer->UpdateVariable(lightColorHandle, glm::vec4(light->GetColor()[0], light->GetColor()[1], light->GetColor()[2], 1.0f));


	//m_GlobalDescriptorSet = Descriptor::DescriptorManager::Allocate(vulkanContext->device, m_GlobalDescriptorSetLayout, 0);
	//m_Writer.Cleanup();

	//m_GlobalBuffer.ProperBind(0, m_Writer);
	//m_Writer.UpdateSet(vulkanContext->device, m_GlobalDescriptorSet);

	//vkCmdBindDescriptorSets(commandBuffer, static_cast<VkPipelineBindPoint>(pipelineType), pipelineLayout, 0, 1, &m_GlobalDescriptorSet, 0, nullptr);

	//TODO: Cleanup this call please
	m_DescriptorSet.Bind(m_pVulkanContext, commandBuffer, m_PipelineLayout, 0,PipelineType::Graphics, true);
}

VkDescriptorSetLayout& GlobalDescriptor::GetLayout()
{
	return m_DescriptorSet.GetLayout(m_pVulkanContext);
}

VkPipelineLayout& GlobalDescriptor::GetPipelineLayout()
{
	return m_PipelineLayout;
}

DescriptorSet& GlobalDescriptor::GetDescriptorSet()
{
	return m_DescriptorSet;
}

void GlobalDescriptor::Cleanup(VkDevice device)
{
	vkDestroyPipelineLayout(device, m_PipelineLayout, nullptr);
}

void GlobalDescriptor::OnImGui()
{
	ImGui::Begin("Global Descriptor");
	LightManager::OnImGui();
	ImGui::End();
}
