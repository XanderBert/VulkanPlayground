#include "Core/GlobalDescriptor.h"

#include "DepthResource.h"
#include "Descriptor.h"
#include "Camera/Camera.h"
#include "shaders/Logic/Shader.h"

void GlobalDescriptor::Init(VulkanContext *vulkanContext)
{
	//TODO: I Need a proper way to pass a ARRAY of light structurs to the GPU.
	const std::vector<Light *> Lights = LightManager::GetLights();
	Light *light = Lights[0];



	m_GlobalBuffer.SetDescriptorType(DescriptorType::UniformBuffer);

	viewProjectionHandle = m_GlobalBuffer.AddVariable(Camera::GetViewProjectionMatrix());
	cameraHandle = m_GlobalBuffer.AddVariable(glm::vec4(Camera::GetPosition(), 1.0f));
	cameraPlaneHandle = m_GlobalBuffer.AddVariable(glm::vec4(Camera::GetNearPlane(), Camera::GetFarPlane(), 0.0f, 0.0f));
	lightPositionHandle = m_GlobalBuffer.AddVariable(glm::vec4(light->GetPosition()[0], light->GetPosition()[1], light->GetPosition()[2], 1.0f));
	lightColorHandle = m_GlobalBuffer.AddVariable(glm::vec4(light->GetColor()[0], light->GetColor()[1], light->GetColor()[2], 1.0f));
	inverseProjectionHandle = m_GlobalBuffer.AddVariable(inverse(Camera::GetProjectionMatrix()));
	viewMatrixHandle = m_GlobalBuffer.AddVariable(Camera::GetViewMatrix());


	m_GlobalBuffer.Init();

	Descriptor::DescriptorBuilder builder{};
	builder.AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
	m_GlobalDescriptorSetLayout = builder.Build(vulkanContext->device, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_COMPUTE_BIT);
}


void GlobalDescriptor::Bind(VulkanContext *vulkanContext, const VkCommandBuffer commandBuffer, const VkPipelineLayout &pipelineLayout, PipelineType pipelineType)
{
	LogAssert(m_GlobalDescriptorSetLayout != VK_NULL_HANDLE, "GlobalDescriptorSetLayout is not initialized", true);
	m_GlobalBuffer.UpdateVariable(viewProjectionHandle, Camera::GetViewProjectionMatrix());
	m_GlobalBuffer.UpdateVariable(viewProjectionHandle, Camera::GetViewProjectionMatrix());
	m_GlobalBuffer.UpdateVariable(cameraHandle, glm::vec4(Camera::GetPosition(), 1.0f));
	m_GlobalBuffer.UpdateVariable(cameraPlaneHandle, glm::vec4(Camera::GetNearPlane(), Camera::GetFarPlane(), 0.0f, 0.0f));
	m_GlobalBuffer.UpdateVariable(inverseProjectionHandle, inverse(Camera::GetProjectionMatrix()));
	m_GlobalBuffer.UpdateVariable(viewMatrixHandle, Camera::GetViewMatrix());

	//TODO: I Need a proper way to pass a ARRAY of light structurs to the GPU.
	const std::vector<Light *> Lights = LightManager::GetLights();
	Light *light = Lights[0];
	m_GlobalBuffer.UpdateVariable(lightPositionHandle, glm::vec4(light->GetPosition()[0], light->GetPosition()[1], light->GetPosition()[2], 1.0f));
	m_GlobalBuffer.UpdateVariable(lightColorHandle, glm::vec4(light->GetColor()[0], light->GetColor()[1], light->GetColor()[2], 1.0f));


	m_GlobalDescriptorSet = Descriptor::DescriptorManager::Allocate(vulkanContext->device, m_GlobalDescriptorSetLayout, 0);
	m_Writer.Cleanup();


	m_GlobalBuffer.ProperBind(0, m_Writer);
	m_Writer.UpdateSet(vulkanContext->device, m_GlobalDescriptorSet);

	vkCmdBindDescriptorSets(commandBuffer, static_cast<VkPipelineBindPoint>(pipelineType), pipelineLayout, 0, 1, &m_GlobalDescriptorSet, 0, nullptr);
}

VkDescriptorSetLayout &GlobalDescriptor::GetLayout()
{
	return m_GlobalDescriptorSetLayout;
}


void GlobalDescriptor::Cleanup(VkDevice device)
{
	vkDestroyDescriptorSetLayout(device, m_GlobalDescriptorSetLayout, nullptr);
	m_GlobalBuffer.Cleanup(device);
}

void GlobalDescriptor::OnImGui()
{
	ImGui::Begin("Global Descriptor");
	LightManager::OnImGui();
	ImGui::End();
}
