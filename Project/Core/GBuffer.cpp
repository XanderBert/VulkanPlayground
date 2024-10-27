#include "GBuffer.h"

#include "ColorAttachment.h"
#include "SwapChain.h"

#include "vulkanbase/VulkanUtil.h"


void GBuffer::Init(const VulkanContext* vulkanContext)
{
	m_DepthAttachment.Init(vulkanContext);
	m_ColorAttachmentNormal.Init(vulkanContext, VK_FORMAT_R16G16B16A16_SFLOAT, VkClearColorValue{{0.5f, 0.5f, 0.5f, 0.0f}});
}

void GBuffer::Cleanup(const VulkanContext *vulkanContext)
{
	m_DepthAttachment.Cleanup(vulkanContext);
	m_ColorAttachmentNormal.Cleanup(vulkanContext);
}

ColorAttachment* GBuffer::GetColorAttachmentNormal()
{
	return &m_ColorAttachmentNormal;
}

DepthAttachment * GBuffer::GetDepthAttachment()
{
	return &m_DepthAttachment;
}

void GBuffer::OnImGui()
{
	ImGui::Begin("GBuffer");
	m_ColorAttachmentNormal.OnImGui();
	m_DepthAttachment.OnImGui();
	ImGui::End();
}
