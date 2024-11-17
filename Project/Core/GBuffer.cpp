#include "GBuffer.h"

#include "ColorAttachment.h"
#include "SwapChain.h"

#include "vulkanbase/VulkanUtil.h"


void GBuffer::Init(const VulkanContext* vulkanContext)
{
	m_DepthAttachment.Init(vulkanContext);


	auto extends = SwapChain::Extends();
	glm::ivec2 extent = {extends.width, extends.height};
	m_ColorAttachmentNormal.Init(vulkanContext, VK_FORMAT_R16G16B16A16_SFLOAT, VkClearColorValue{{0.5f, 0.5f, 0.5f, 0.0f}}, extent);
	m_AlbedoAttachment.Init(vulkanContext, VK_FORMAT_R8G8B8A8_UNORM, VkClearColorValue{{0.f, 0.f, 0.f, 0.0f}}, extent);


	SwapChain::OnSwapChainRecreated.AddLambda([](const VulkanContext* vulkanContext)
	{
		auto extends = SwapChain::Extends();
		glm::ivec2 extent = {extends.width, extends.height};
		m_ColorAttachmentNormal.Recreate(vulkanContext, VkClearColorValue{{0.5f, 0.5f, 0.5f, 0.0f}}, extent);
		m_AlbedoAttachment.Recreate(vulkanContext, VkClearColorValue{{0.f, 0.f, 0.f, 0.0f}}, extent);
	});
}

void GBuffer::Cleanup(const VulkanContext *vulkanContext)
{
	m_DepthAttachment.Cleanup(vulkanContext);
	m_ColorAttachmentNormal.Cleanup(vulkanContext->device);
	m_AlbedoAttachment.Cleanup(vulkanContext->device);
}

void GBuffer::Bind(Descriptor::DescriptorWriter &writer, int depthBinding, int normalBinding)
{
	m_DepthAttachment.Bind(writer, depthBinding);
	m_ColorAttachmentNormal.Bind(writer, normalBinding);
}

void GBuffer::BindAlbedo(Descriptor::DescriptorWriter &writer, int albedoBinding)
{
	m_AlbedoAttachment.Bind(writer, albedoBinding);
}

ColorAttachment* GBuffer::GetColorAttachmentNormal()
{
	return &m_ColorAttachmentNormal;
}

DepthAttachment * GBuffer::GetDepthAttachment()
{
	return &m_DepthAttachment;
}

ColorAttachment * GBuffer::GetAlbedoAttachment()
{
	return &m_AlbedoAttachment;
}

void GBuffer::OnImGui()
{
	ImGui::Begin("GBuffer");
	m_ColorAttachmentNormal.OnImGui();
	//m_DepthAttachment.OnImGui();
	m_AlbedoAttachment.OnImGui();
	ImGui::End();
}
