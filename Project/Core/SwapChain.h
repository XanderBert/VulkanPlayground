#pragma once

#include <vector>
#include <algorithm>

#include "vulkanbase/VulkanTypes.h"
#include "QueueFamilyIndices.h"
#include "Core/Image/Texture.h"


class SwapChain final
{
public:
	SwapChain() = default;
	~SwapChain() = default;

	SwapChain(const SwapChain&) = delete;
	SwapChain(SwapChain&&) = delete;
	SwapChain& operator=(const SwapChain&) = delete;
	SwapChain& operator=(SwapChain&&) = delete;

    inline static Delegate<const VulkanContext*> OnSwapChainRecreated;

	static VkSwapchainKHR GetSwapChain() { return m_SwapChain; }

	static VkImage& Image(int index) { return m_SwapChainImages[index]; }

	static void Init(const VulkanContext* vulkanContext)
	{
		const VkPhysicalDevice physicalDevice = vulkanContext->physicalDevice;
		const VkDevice device = vulkanContext->device;

		const SwapChainSupportDetails swapChainSupport = GetSupportDetails(physicalDevice);
		const VkSurfaceFormatKHR surfaceFormat = GetSurfaceFormat(swapChainSupport.formats);
		const VkPresentModeKHR presentMode = GetPresentMode(swapChainSupport.presentModes);
		const VkExtent2D extent = GetExtends(swapChainSupport.capabilities, vulkanContext);


		uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
		if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) 
		{
			imageCount = swapChainSupport.capabilities.maxImageCount;
		}

		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = m_SwapChainSurface;

		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;


		const QueueFamilyIndices indices = QueueFamilyIndices::FindQueueFamilies(physicalDevice, m_SwapChainSurface);
		const uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

		if (indices.graphicsFamily != indices.presentFamily)
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		}

		createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;

		createInfo.oldSwapchain = VK_NULL_HANDLE;

		VulkanCheck(vkCreateSwapchainKHR(device, &createInfo, nullptr, &m_SwapChain), "Failed To Create SwapChain")

		vkGetSwapchainImagesKHR(device, m_SwapChain, &imageCount, nullptr);
		m_SwapChainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(device, m_SwapChain, &imageCount, m_SwapChainImages.data());

		m_Format = surfaceFormat.format;
		m_Extends = extent;

		CreateImageViews(device);
	}

	static void CreateSurface(const VulkanContext* vulkanContext)
	{
		VulkanCheck(glfwCreateWindowSurface(vulkanContext->instance, vulkanContext->window.Ptr(), nullptr, &m_SwapChainSurface), "Failed To Create Window Surface")
	}

	static void DestroySwapChain(const VulkanContext* vulkanContext)
	{
		for (const auto imageView : m_SwapChainImageViews)
		{
			vkDestroyImageView(vulkanContext->device, imageView, nullptr);
		}
		vkDestroySwapchainKHR(vulkanContext->device, m_SwapChain, nullptr);
	}

	static void Cleanup(const VulkanContext* vulkanContext)
	{
		DestroySwapChain(vulkanContext);
		vkDestroySurfaceKHR(vulkanContext->instance, m_SwapChainSurface, nullptr);
	}

	static void SetNeedsRecreation() { m_NeedsRecreation = true; }
	static VkSurfaceKHR& GetSurface() { return m_SwapChainSurface; }
	static std::vector<VkImageView>& ImageViews() { return m_SwapChainImageViews; }
	static VkExtent2D& Extends() { return m_Extends; }
	static VkFormat& Format() { return m_Format; }
	static uint8_t ImageCount() { return static_cast<uint8_t>(m_SwapChainImages.size()); }

	static void RecreateIfNeeded(const VulkanContext* vulkanContext)
	{
		if (!m_NeedsRecreation) return;

	    LogInfo("Recreating SwapChain");
		DestroySwapChain(vulkanContext);
		Init(vulkanContext);
		vkDeviceWaitIdle(vulkanContext->device);

	    OnSwapChainRecreated.Broadcast(vulkanContext);
		m_NeedsRecreation = false;
	}

private:
	struct SwapChainSupportDetails
	{
		VkSurfaceCapabilitiesKHR capabilities{};
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};



	static SwapChainSupportDetails GetSupportDetails(VkPhysicalDevice device)
	{
		LogAssert(m_SwapChainSurface != VK_NULL_HANDLE, "Swap chain surface is not initialized", true)

		SwapChainSupportDetails details;

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_SwapChainSurface, &details.capabilities);

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_SwapChainSurface, &formatCount, nullptr);

		if (formatCount != 0)
		{
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_SwapChainSurface, &formatCount, details.formats.data());
		}

		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_SwapChainSurface, &presentModeCount, nullptr);

		if (presentModeCount != 0)
		{
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_SwapChainSurface, &presentModeCount, details.presentModes.data());
		}

		return details;
	}

	static VkSurfaceFormatKHR GetSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
	{
		for (const auto& availableFormat : availableFormats) 
		{
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) 
			{
				return availableFormat;
			}
		}

		return availableFormats[0];
	}

	static VkPresentModeKHR GetPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
	{
		for (const auto& availablePresentMode : availablePresentModes) 
		{
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) 
			{
				return availablePresentMode;
			}
		}

		return VK_PRESENT_MODE_FIFO_KHR;
	}

	static VkExtent2D GetExtends(const VkSurfaceCapabilitiesKHR& capabilities, const VulkanContext* vulkanContext)
	{
		//If the extent is not defined, the window size will be used
		if (capabilities.currentExtent.width != UINT32_MAX) return capabilities.currentExtent;

		int width = 0, height = 0;
		vulkanContext->window.GetSize(width, height);
		while (width == 0 || height == 0) 
		{
			vulkanContext->window.GetSize(width, height);
			glfwWaitEvents();
		}

		VkExtent2D actualExtent = 
		{
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		return actualExtent;
	}

	static void CreateImageViews(VkDevice device)
	{
		m_SwapChainImageViews.resize(m_SwapChainImages.size());

		for (size_t i = 0; i < m_SwapChainImages.size(); ++i)
		{
			Image::CreateImageView(device, m_SwapChainImages[i], m_Format, VK_IMAGE_ASPECT_COLOR_BIT, m_SwapChainImageViews[i], TextureType::TEXTURE_2D);
		}
	}

	inline static bool m_NeedsRecreation = false;

	inline static VkSwapchainKHR m_SwapChain{};
	inline static VkExtent2D m_Extends;
	inline static VkFormat m_Format;
	inline static VkSurfaceKHR m_SwapChainSurface;

	inline static std::vector<VkImage> m_SwapChainImages;
	inline static std::vector<VkImageView> m_SwapChainImageViews;
};
