#include "ColorAttachment.h"

#include "SwapChain.h"
#include "Image/ImageLoader.h"
#include "vulkanbase/VulkanUtil.h"

void ColorAttachment::Init(const VulkanContext *vulkanContext, VkFormat format, VkClearColorValue clearColor, const glm::ivec2 &extent)
{
	m_Format = format;
	Setup(vulkanContext, clearColor, extent);
}

void ColorAttachment::Recreate(const VulkanContext *vulkanContext, VkClearColorValue clearColor, const glm::ivec2 &extent)
{
	Cleanup(vulkanContext->device);
	Setup(vulkanContext, clearColor, extent);
}

void ColorAttachment::Cleanup(VkDevice device)
{
	vkDestroyImageView(device, m_ImageView, nullptr);
	vmaDestroyImage(Allocator::VmaAllocator, m_Image, m_Memory);

	if(m_Sampler != VK_NULL_HANDLE) vkDestroySampler(device, m_Sampler, nullptr);

	if(m_DebugTexture)
	{
		m_DebugTexture->Cleanup();
		m_DebugTexture.reset();
	}
}

//Only Bind if we will use tha attachment in a descriptor set -> If it will need a binding number
void ColorAttachment::Bind(Descriptor::DescriptorWriter &writer, int bindingNumber) const
{
	writer.WriteImage(bindingNumber, m_ImageView, m_Sampler, m_CurrentImageLayout, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
}

VkRenderingAttachmentInfoKHR* ColorAttachment::GetRenderingAttachmentInfo()
{
	return &m_ColorAttachmentInfo;
}

VkFormat* ColorAttachment::GetFormat()
{
	return &m_Format;
}

void ColorAttachment::ResetImageLayout()
{
	m_CurrentImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
}

void ColorAttachment::TransitionToWrite(VkCommandBuffer commandBuffer)
{
	tools::InsertImageMemoryBarrier(
   commandBuffer,
   m_Image,
   m_CurrentImageLayout,
   VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
   VkImageSubresourceRange{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1});

	m_CurrentImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
}

void ColorAttachment::TransitionToRead(VkCommandBuffer commandBuffer)
{
	tools::InsertImageMemoryBarrier(
	   commandBuffer,
	   m_Image,
	   m_CurrentImageLayout,
	   VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
	   VkImageSubresourceRange{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1});

	m_CurrentImageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
}

void ColorAttachment::TransitionToGeneralResource(VkCommandBuffer commandBuffer)
{
	tools::InsertImageMemoryBarrier(
	   commandBuffer,
	   m_Image,
	   m_CurrentImageLayout,
	   VK_IMAGE_LAYOUT_GENERAL,
	   VkImageSubresourceRange{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1});

	m_CurrentImageLayout = VK_IMAGE_LAYOUT_GENERAL;
}

void ColorAttachment::OnImGui()
{
	if(!m_DebugTexture.get())
	{
		//Create a new updated ImGuiTexture for the depthmap
		const auto extends = SwapChain::Extends();
		m_DebugTexture = std::make_unique<ImGuiTexture>(m_Sampler, m_ImageView, ImVec2(extends.width / 5.0f, extends.height / 5.0f));
	}

	m_DebugTexture->OnImGui();
}

void ColorAttachment::Setup(const VulkanContext *vulkanContext, VkClearColorValue clearColor, const glm::ivec2 &extent)
{
	m_CurrentImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	Image::CreateImage(extent.x, extent.y,
		1,
		VK_SAMPLE_COUNT_1_BIT,
		m_Format,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		m_Image, m_Memory, TextureType::TEXTURE_2D);

	Image::CreateImageView(vulkanContext->device, m_Image, m_Format, VK_IMAGE_ASPECT_COLOR_BIT, m_ImageView, TextureType::TEXTURE_2D);
	Image::CreateSampler(vulkanContext, m_Sampler, 1);

	m_ColorAttachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
	m_ColorAttachmentInfo.pNext = nullptr;
	m_ColorAttachmentInfo.imageView = m_ImageView;
	m_ColorAttachmentInfo.imageLayout = m_CurrentImageLayout;
	m_ColorAttachmentInfo.clearValue.color = clearColor;
	m_ColorAttachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	m_ColorAttachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
}
