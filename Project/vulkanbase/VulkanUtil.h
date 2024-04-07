#pragma once
#include <string>
#include <vulkan/vulkan.h>
#include <vector>


#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif


class VulkanContext;
namespace tools
{
	VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);

	void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);

	std::vector<char> readFile(const std::string& filename);

	std::string readFileStr(const std::string& filename);

	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties, VkPhysicalDevice physicalDevice);

	VkAccessFlags GetAccessFlags(VkImageLayout layout);

	VkPipelineStageFlags GetPipelineStageFlags(VkImageLayout layout);

	void InsertImageMemoryBarrier(const VkCommandBuffer commandBuffer, VkImage image, VkAccessFlags srcAccessMask,VkAccessFlags dstAccessMask, VkImageLayout oldImageLayout, VkImageLayout newImageLayout, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkImageSubresourceRange subresourceRange);

	void InsertImageMemoryBarrier(VkCommandBuffer commandBuffer,
		VkImage                        image,
		VkImageLayout                  oldLayout,
		VkImageLayout                  newLayout,
		VkImageSubresourceRange const& subresourceRange);

	void CreateImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, VkImageView& imageView);

	bool HasStencilComponent(VkFormat format);
}
