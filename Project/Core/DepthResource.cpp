#include "DepthResource.h"

#include "ImGuiWrapper.h"
#include "Logger.h"
#include "SwapChain.h"

void DepthResource::Recreate(const VulkanContext* vulkanContext)
{
	Cleanup(vulkanContext);
    DepthResourceBuilder::Build(vulkanContext, m_Image, m_ImageView, m_Memory, m_Format);
}

void DepthResource::Init(const VulkanContext* vulkanContext)
{
	DepthResourceBuilder::Build(vulkanContext, m_Image, m_ImageView, m_Memory, m_Format);
    SwapChain::OnSwapChainRecreated.AddLambda([&](const VulkanContext* vulkanContext)
    {
        Recreate(vulkanContext);
    });
}

void DepthResource::Cleanup(const VulkanContext* vulkanContext)
{
	vkDestroyImageView(vulkanContext->device, m_ImageView, nullptr);
    vmaDestroyImage(Allocator::VmaAllocator, m_Image, m_Memory);
}

VkPipelineDepthStencilStateCreateInfo DepthResource::GetDepthPipelineInfo(VkBool32 depthTestEnable, VkBool32 depthWriteEnable)
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

VkImageView DepthResource::GetImageView()
{
	return m_ImageView;
}

VkFormat DepthResource::GetFormat()
{
	return m_Format;
}

VkImage DepthResource::GetImage() { return m_Image; }

void DepthResourceBuilder::Build(const VulkanContext* vulkanContext, VkImage& image, VkImageView& imageView , VmaAllocation& memory, VkFormat& format)
{
    format = FindDepthFormat(vulkanContext);
    CreateDepthResources(vulkanContext, image, imageView, memory);
}

VkFormat DepthResourceBuilder::FindDepthFormat(const VulkanContext* vulkanContext)
{
	const std::vector<VkFormat>& depthFormatPriorityList =
	{
		VK_FORMAT_D32_SFLOAT,
		VK_FORMAT_D24_UNORM_S8_UINT,
		VK_FORMAT_D16_UNORM
	};

	return FindSupportedFormat(vulkanContext, depthFormatPriorityList, true);
}

void DepthResourceBuilder::CreateDepthResources(const VulkanContext* vulkanContext, VkImage& image, VkImageView& imageView , VmaAllocation& memory)
{
	const VkFormat depthFormat = FindDepthFormat(vulkanContext);

	//Create Image
    //VK_IMAGE_USAGE_SAMPLED_BIT must be added to allow the depth image to be used as a texture (Shadow Mapping)
	Image::CreateImage(SwapChain::Extends().width, SwapChain::Extends().height, 1, VK_SAMPLE_COUNT_1_BIT, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, image, memory, TextureType::TEXTURE_2D);


	VkImageAspectFlags aspectMaskFlags = VK_IMAGE_ASPECT_DEPTH_BIT;

	// Stencil aspect should only be set on depth + stencil formats (VK_FORMAT_D16_UNORM_S8_UINT..VK_FORMAT_D32_SFLOAT_S8_UINT
	if (depthFormat >= VK_FORMAT_D16_UNORM_S8_UINT)
	{
		aspectMaskFlags |= VK_IMAGE_ASPECT_STENCIL_BIT;
	}

	Image::CreateImageView(vulkanContext->device, image, depthFormat, aspectMaskFlags, imageView, TextureType::TEXTURE_2D);
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
