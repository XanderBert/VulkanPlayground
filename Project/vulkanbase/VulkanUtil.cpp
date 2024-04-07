#include "VulkanUtil.h"
#include <fstream>
#include "VulkanTypes.h"
#include "Core/CommandBuffer.h"
#include "Core/Logger.h"

namespace tools
{
	VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
		auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
		if (func != nullptr) {
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		}
		else {
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
		auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
			vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
		if (func != nullptr) {
			func(instance, debugMessenger, pAllocator);
		}
	}

	std::vector<char> readFile(const std::string& filename)
	{
		std::ifstream file(filename, std::ios::ate | std::ios::binary);

		if (!file.is_open())
		{
			throw std::runtime_error("failed to open file!");
		}

		//Get the size of the file and use it to allocate a buffer
		const size_t fileSize = (size_t)file.tellg();
		std::vector<char> buffer(fileSize);

		//Go to the beginning of the file and read all the bytes at once
		file.seekg(0);
		file.read(buffer.data(), fileSize);

		//Close the file and return the buffer filled with the file's contents
		file.close();
		return buffer;
	}

	std::string readFileStr(const std::string& filename)
	{
		std::vector<char> fileData = readFile(filename);
		return std::string{ fileData.begin(), fileData.end() };
	}

	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties, VkPhysicalDevice physicalDevice)
	{
		//find info about the available memory types
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);


		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
		{
			//typeFilter parameter will be used to specify the bit field of memory types that are suitable.
			//so we can check if the memory type i is suitable by checking if the bit is set to 1

			//We also need to check if the memory type is suitable for the buffer that we are trying to create
			//We can do this by checking the memoryTypeBits field against the properties that we need
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
			{
				return i;
			}
		}

		throw std::runtime_error("failed to find suitable memory type!");
	}

	VkAccessFlags GetAccessFlags(VkImageLayout layout)
	{
		switch (layout)
		{
		case VK_IMAGE_LAYOUT_UNDEFINED:
		case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
			return 0;
		case VK_IMAGE_LAYOUT_PREINITIALIZED:
			return VK_ACCESS_HOST_WRITE_BIT;
		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			return VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
		case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL:
			return VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		case VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR:
			return VK_ACCESS_FRAGMENT_SHADING_RATE_ATTACHMENT_READ_BIT_KHR;
		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			return VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
			return VK_ACCESS_TRANSFER_READ_BIT;
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			return VK_ACCESS_TRANSFER_WRITE_BIT;
		case VK_IMAGE_LAYOUT_GENERAL:
			LogError("Don't know how to get a meaningful VkAccessFlags for VK_IMAGE_LAYOUT_GENERAL! Don't use it!");
			return 0;
		}
		return 0;
	}

	VkPipelineStageFlags GetPipelineStageFlags(VkImageLayout layout)
	{
		switch (layout)
		{
		case VK_IMAGE_LAYOUT_UNDEFINED:
			return VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		case VK_IMAGE_LAYOUT_PREINITIALIZED:
			return VK_PIPELINE_STAGE_HOST_BIT;
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
			return VK_PIPELINE_STAGE_TRANSFER_BIT;
		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			return VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
		case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL:
			return VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		case VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR:
			return VK_PIPELINE_STAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR;
		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			return VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
			return VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		case VK_IMAGE_LAYOUT_GENERAL:
			LogError("Don't know how to get a meaningful VkPipelineStageFlags for VK_IMAGE_LAYOUT_GENERAL! Don't use it!");
			return 0;
		}
		return 0;
	}

	void InsertImageMemoryBarrier(const VkCommandBuffer commandBuffer, VkImage image, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, VkImageLayout oldImageLayout, VkImageLayout newImageLayout, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkImageSubresourceRange subresourceRange)
	{
		VkImageMemoryBarrier imageMemoryBarrier{};
		imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.srcAccessMask = srcAccessMask;
		imageMemoryBarrier.dstAccessMask = dstAccessMask;
		imageMemoryBarrier.oldLayout = oldImageLayout;
		imageMemoryBarrier.newLayout = newImageLayout;
		imageMemoryBarrier.image = image;
		imageMemoryBarrier.subresourceRange = subresourceRange;

		vkCmdPipelineBarrier(
			commandBuffer,
			srcStageMask,
			dstStageMask,
			0,
			0, nullptr,
			0, nullptr,
			1, &imageMemoryBarrier);
	}


	void InsertImageMemoryBarrier(VkCommandBuffer commandBuffer,
		VkImage                        image,
		VkImageLayout                  oldLayout,
		VkImageLayout                  newLayout,
		VkImageSubresourceRange const& subresourceRange)
	{
		const VkPipelineStageFlags src_stage_mask = GetPipelineStageFlags(oldLayout);
		const VkPipelineStageFlags dst_stage_mask = GetPipelineStageFlags(newLayout);
		const VkAccessFlags        src_access_mask = GetAccessFlags(oldLayout);
		const VkAccessFlags        dst_access_mask = GetAccessFlags(newLayout);
		
		InsertImageMemoryBarrier(commandBuffer, image, src_access_mask, dst_access_mask , oldLayout, newLayout, src_stage_mask, dst_stage_mask, subresourceRange);
	}


	void CreateImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, VkImageView& imageView)
	{
		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = format;

		viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		viewInfo.subresourceRange.aspectMask = aspectFlags;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		VulkanCheck(vkCreateImageView(device, &viewInfo, nullptr, &imageView), "Failed to create texture image view!")
	}

	bool HasStencilComponent(VkFormat format)
	{
		return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
	}

}
