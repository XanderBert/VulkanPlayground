#include "DepthResource.h"

#include "ImGuiWrapper.h"
#include "Logger.h"
#include "SwapChain.h"
#include "Camera/Camera.h"
#include "vulkanbase/VulkanUtil.h"

void DepthAttachment::Recreate(const VulkanContext* vulkanContext)
{
	Cleanup(vulkanContext);
    DepthResourceBuilder::Build(vulkanContext, m_Image, m_ImageView, m_Memory, m_Format);
    Image::CreateSampler(vulkanContext, m_Sampler, 1);


	m_DepthAttachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
	m_DepthAttachmentInfo.imageView = m_ImageView;
	m_DepthAttachmentInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	m_DepthAttachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	m_DepthAttachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	m_DepthAttachmentInfo.clearValue = {{1.0f, 0.0f}};
}

void DepthAttachment::Init(const VulkanContext* vulkanContext)
{
	DepthResourceBuilder::Build(vulkanContext, m_Image, m_ImageView, m_Memory, m_Format);
    SwapChain::OnSwapChainRecreated.AddLambda([&](const VulkanContext* vulkanContext)
    {
        Recreate(vulkanContext);
    });

    Image::CreateSampler(vulkanContext, m_Sampler, 1);

	m_DepthAttachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
	m_DepthAttachmentInfo.imageView = m_ImageView;
	m_DepthAttachmentInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	m_DepthAttachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	m_DepthAttachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	m_DepthAttachmentInfo.clearValue = {{1.0f, 0.0f}};
}

void DepthAttachment::Cleanup(const VulkanContext* vulkanContext)
{
	vkDestroyImageView(vulkanContext->device, m_ImageView, nullptr);
    vmaDestroyImage(Allocator::vmaAllocator, m_Image, m_Memory);

    if(m_Sampler != VK_NULL_HANDLE)
    vkDestroySampler(vulkanContext->device, m_Sampler, nullptr);

	if(m_ImGuiTexture)
	{
		m_ImGuiTexture->Cleanup();
		m_ImGuiTexture.reset();
	}
}

void DepthAttachment::Bind(Descriptor::DescriptorWriter &writer, int bindingNumber) const
{
	writer.WriteImage(bindingNumber, m_ImageView, m_Sampler, m_BindImageLayout, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
}



VkRenderingAttachmentInfoKHR * DepthAttachment::GetRenderingAttachmentInfo()
{
	return &m_DepthAttachmentInfo;
}

VkPipelineDepthStencilStateCreateInfo DepthAttachment::GetDepthPipelineInfo(VkBool32 depthTestEnable, VkBool32 depthWriteEnable)
{
	VkPipelineDepthStencilStateCreateInfo depthPipelineInfo{};
	depthPipelineInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthPipelineInfo.depthTestEnable = depthTestEnable;
	depthPipelineInfo.depthWriteEnable = depthWriteEnable;
	depthPipelineInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	depthPipelineInfo.front = depthPipelineInfo.back;
	depthPipelineInfo.back.compareOp = VK_COMPARE_OP_ALWAYS;

	return depthPipelineInfo;
}

void DepthAttachment::OnImGui()
{
	if(!m_ImGuiTexture)
	{
		//Create a new updated ImGuiTexture for the depthmap
		const auto extends = SwapChain::Extends();
		m_ImGuiTexture = std::make_unique<ImGuiTexture>(m_Sampler, m_ImageView, ImVec2(extends.width / 5.0f, extends.height / 5.0f));
	}

	m_ImGuiTexture->OnImGui();
}

void DepthAttachment::TransitionToDepthResource(VkCommandBuffer commandBuffer)
{
	m_DepthAttachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	m_DepthAttachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;

    tools::InsertImageMemoryBarrier(
        commandBuffer,
        m_Image,
        m_BindImageLayout,
        VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
        VkImageSubresourceRange{VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1});


    m_BindImageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
}

void DepthAttachment::TransitionToShaderRead(VkCommandBuffer commandBuffer)
{
	m_DepthAttachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	m_DepthAttachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;

	tools::InsertImageMemoryBarrier(
		commandBuffer,
		m_Image,
		m_BindImageLayout,
		VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
		VkImageSubresourceRange{VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1});


	m_BindImageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
}

void DepthAttachment::TransitionToShaderReadOnly(VkCommandBuffer commandBuffer)
{
	m_DepthAttachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	m_DepthAttachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;


	tools::InsertImageMemoryBarrier(
		commandBuffer,
		m_Image,
		m_BindImageLayout,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		VkImageSubresourceRange{VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1});


	m_BindImageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
}

void DepthAttachment::TransitionToGeneralResource(VkCommandBuffer commandBuffer)
{
	m_DepthAttachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	m_DepthAttachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;

    tools::InsertImageMemoryBarrier(
       commandBuffer,
       m_Image,
       m_BindImageLayout,
       VK_IMAGE_LAYOUT_GENERAL,
       VkImageSubresourceRange{VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1});

    m_BindImageLayout = VK_IMAGE_LAYOUT_GENERAL;
}

void DepthAttachment::ResetImageLayout() { m_BindImageLayout = VK_IMAGE_LAYOUT_UNDEFINED; }

VkImageLayout DepthAttachment::GetBindImageLayout()
{
    return m_BindImageLayout;
}

VkFormat DepthAttachment::GetFormat() const
{
	return m_Format;
}

VkImage DepthAttachment::GetImage() const
{
	return m_Image;
}


void DepthResourceBuilder::Build(const VulkanContext* vulkanContext, VkImage& image, VkImageView& imageView , VmaAllocation& memory, VkFormat& format)
{
    format = FindDepthFormat(vulkanContext);
    CreateDepthResources(vulkanContext, image, imageView, memory, format);
}

VkFormat DepthResourceBuilder::FindDepthFormat(const VulkanContext* vulkanContext)
{
	const std::vector<VkFormat>& depthFormatPriorityList =
	{
		VK_FORMAT_R16_UINT,
		VK_FORMAT_D32_SFLOAT,
		VK_FORMAT_D24_UNORM_S8_UINT,
		VK_FORMAT_D16_UNORM,
	};

	return FindSupportedFormat(vulkanContext, depthFormatPriorityList, true);
}

void DepthResourceBuilder::CreateDepthResources(const VulkanContext* vulkanContext, VkImage& image, VkImageView& imageView , VmaAllocation& memory, const VkFormat& format)
{
	//Create Image
    //VK_IMAGE_USAGE_SAMPLED_BIT must be added to allow the depth image to be used as a texture (Shadow Mapping)
	Image::CreateImage(SwapChain::Extends().width, SwapChain::Extends().height, 1, VK_SAMPLE_COUNT_1_BIT, format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, image, memory, TextureType::TEXTURE_2D);


	VkImageAspectFlags aspectMaskFlags = VK_IMAGE_ASPECT_DEPTH_BIT;

	// Stencil aspect should only be set on depth + stencil formats (VK_FORMAT_D16_UNORM_S8_UINT..VK_FORMAT_D32_SFLOAT_S8_UINT
	if (format >= VK_FORMAT_D16_UNORM_S8_UINT)
	{
		aspectMaskFlags |= VK_IMAGE_ASPECT_STENCIL_BIT;
	}

	Image::CreateImageView(vulkanContext->device, image, format, aspectMaskFlags, imageView, TextureType::TEXTURE_2D);
}

VkFormat DepthResourceBuilder::FindSupportedFormat(const VulkanContext* vulkanContext, const std::vector<VkFormat>& candidates, bool isDepthOnly)
{
	VkFormat depth_format{ VK_FORMAT_UNDEFINED };

	for (auto& format : candidates)
	{
		if (isDepthOnly && Image::HasStencilComponent(format))
		{
			continue;
		}

		VkFormatProperties properties;
		vkGetPhysicalDeviceFormatProperties(vulkanContext->physicalDevice, format, &properties);

		// Format must support depth stencil attachment for optimal tiling
		if (properties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
		{
			depth_format = format;
			break;
		}
	}

	if (depth_format != VK_FORMAT_UNDEFINED) return depth_format;


	LogError("No suitable depth format could be determined");
	return {};
}
