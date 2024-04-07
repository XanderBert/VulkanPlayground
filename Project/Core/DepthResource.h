#pragma once
#include <vulkan/vulkan.h>
#include "vulkanbase/VulkanTypes.h"
#include "Logger.h"

namespace DepthResource 
{
	inline VkFormat FindSupportedFormat(const VulkanContext* vulkanContext, const std::vector<VkFormat>& candidates, bool isDepthOnly)
	{
		VkFormat depth_format{ VK_FORMAT_UNDEFINED };

		for (auto& format : candidates)
		{
			if (isDepthOnly && tools::HasStencilComponent(format))
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

	class DepthResource
	{
	public:

		static void Init(const VulkanContext* vulkanContext)
		{
			m_Format = FindDepthFormat(vulkanContext);
			CreateDepthResources(vulkanContext);
		}

		static void Cleanup(VulkanContext* vulkanContext)
		{
			vkDestroyImageView(vulkanContext->device, m_ImageView, nullptr);
			vkDestroyImage(vulkanContext->device, m_Image, nullptr);
			vkFreeMemory(vulkanContext->device, m_Memory, nullptr);
		}

		static  VkPipelineDepthStencilStateCreateInfo GetDepthPipelineInfo(VkBool32 depthTestEnable, VkBool32 depthWriteEnable)
		{
			VkPipelineDepthStencilStateCreateInfo depthPipelineInfo{};
			depthPipelineInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
			depthPipelineInfo.depthTestEnable = depthTestEnable;
			depthPipelineInfo.depthWriteEnable = depthWriteEnable;
			depthPipelineInfo.depthCompareOp = VK_COMPARE_OP_LESS;
			depthPipelineInfo.front = depthPipelineInfo.back;
			depthPipelineInfo.back.compareOp = VK_COMPARE_OP_ALWAYS;

			return depthPipelineInfo;
		}

		static VkImageView GetImageView()
		{
#ifdef _DEBUG
			LogAssert(m_ImageView != VK_NULL_HANDLE, "Depth Resource Image View is Null", true)
#endif
			return m_ImageView;
		}

		static VkFormat GetFormat()
		{
#ifdef _DEBUG
			LogAssert(m_Format != VK_FORMAT_UNDEFINED, "Depth Resource Format is Undefined", true)
#endif
			return m_Format;
		}

		static VkImage GetImage()
		{
#ifdef _DEBUG
			LogAssert(m_Image != VK_NULL_HANDLE, "Depth Resource Image is Null", true)
#endif // DEBUG

			return m_Image;
		}


	private:
		inline static VkImage m_Image;
		inline static VkDeviceMemory m_Memory;
		inline static VkImageView m_ImageView;
		inline static VkFormat m_Format;


		//----Functions----
		static VkFormat FindDepthFormat(const VulkanContext* vulkanContext)
		{
			const std::vector<VkFormat>& depthFormatPriorityList =
			{
				VK_FORMAT_D32_SFLOAT,
				VK_FORMAT_D24_UNORM_S8_UINT,
				VK_FORMAT_D16_UNORM
			};

			return FindSupportedFormat(vulkanContext, depthFormatPriorityList, true);
		}

		static  void CreateDepthResources(const VulkanContext* vulkanContext)
		{
			const VkFormat depthFormat = FindDepthFormat(vulkanContext);

			//Create Image
			VkImageCreateInfo image_create_info{};
			image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			image_create_info.imageType = VK_IMAGE_TYPE_2D;
			image_create_info.format = depthFormat;
			image_create_info.extent = { vulkanContext->swapChainExtent.width, vulkanContext->swapChainExtent.width, 1 };
			image_create_info.mipLevels = 1;
			image_create_info.arrayLayers = 1;
			image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
			image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
			image_create_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

			VulkanCheck(vkCreateImage(vulkanContext->device, &image_create_info, nullptr, &m_Image), "Failed to Create Depth Image")


			VkMemoryRequirements memReqs{};
			vkGetImageMemoryRequirements(vulkanContext->device, m_Image, &memReqs);

			VkMemoryAllocateInfo memory_allocation{};
			memory_allocation.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			memory_allocation.allocationSize = memReqs.size;
			memory_allocation.memoryTypeIndex = tools::findMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vulkanContext->physicalDevice);

			VulkanCheck(vkAllocateMemory(vulkanContext->device, &memory_allocation, nullptr, &m_Memory), "Failed to Allocate Memory for the depth image")
			VulkanCheck(vkBindImageMemory(vulkanContext->device, m_Image, m_Memory, 0), "Failed to Bind Image Memory")

			VkImageAspectFlags aspectMaskFlags = VK_IMAGE_ASPECT_DEPTH_BIT;

			// Stencil aspect should only be set on depth + stencil formats (VK_FORMAT_D16_UNORM_S8_UINT..VK_FORMAT_D32_SFLOAT_S8_UINT
			if (depthFormat >= VK_FORMAT_D16_UNORM_S8_UINT)
			{
				aspectMaskFlags |= VK_IMAGE_ASPECT_STENCIL_BIT;
			}

			tools::CreateImageView(vulkanContext->device, m_Image, depthFormat, aspectMaskFlags, m_ImageView);
		}
	};

}