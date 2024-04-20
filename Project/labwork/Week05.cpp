#include <set>

#include "vulkanbase/VulkanBase.h"

void VulkanBase::pickPhysicalDevice()
{
	uint32_t deviceCount = 0;

	vkEnumeratePhysicalDevices(m_pContext->instance, &deviceCount, nullptr);
	LogAssert(deviceCount > 0, "failed to find GPUs with Vulkan support!", true);


	std::vector<VkPhysicalDevice> devices{ deviceCount };
	vkEnumeratePhysicalDevices(m_pContext->instance, &deviceCount, devices.data());

	LogAssert(deviceCount > 0, "failed to find GPUs with Vulkan support!", true);


	for (const auto& device : devices) 
	{
		if (isDeviceSuitable(device)) 
		{ 
			m_pContext->physicalDevice = device;
			break;
		}
	}

	LogAssert(m_pContext->physicalDevice != VK_NULL_HANDLE, "failed to find a suitable GPU!", true);
}

bool VulkanBase::isDeviceSuitable(VkPhysicalDevice device)
{
	VkPhysicalDeviceFeatures supportedFeatures;
	vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

	const QueueFamilyIndices indices = QueueFamilyIndices::FindQueueFamilies(device, SwapChain::GetSurface());

	const bool extensionsSupported = checkDeviceExtensionSupport(device);

	return indices.isComplete() && extensionsSupported && supportedFeatures.samplerAnisotropy;
}

void VulkanBase::createLogicalDevice()
{
	VkPhysicalDevice physicalDevice = m_pContext->physicalDevice;
	
	QueueFamilyIndices indices = QueueFamilyIndices::FindQueueFamilies(physicalDevice, SwapChain::GetSurface());

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

	float queuePriority = 1.0f;
	for (uint32_t queueFamily : uniqueQueueFamilies) {
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	VkDeviceQueueCreateInfo queueCreateInfo{};
	queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfo.queueFamilyIndex = indices.graphicsFamily.value();
	queueCreateInfo.queueCount = 1;


	//Set the device features
	VkPhysicalDeviceFeatures deviceFeatures{};
	deviceFeatures.samplerAnisotropy = VK_TRUE;

	//Set the Dynamic Rendering Extension
	VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamicRenderingFeature{};
	dynamicRenderingFeature.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR;
	dynamicRenderingFeature.dynamicRendering = VK_TRUE;


	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.pEnabledFeatures = &deviceFeatures;
	createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	createInfo.ppEnabledExtensionNames = deviceExtensions.data();
	createInfo.pNext = &dynamicRenderingFeature;

	if (enableValidationLayers) 
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}

	createInfo.enabledLayerCount = 0;
	

	
	VulkanCheck(vkCreateDevice(physicalDevice, &createInfo, nullptr, &m_pContext->device), "failed to create logical device!");

	vkGetDeviceQueue(m_pContext->device, indices.graphicsFamily.value(), 0, &m_pContext->graphicsQueue);
	vkGetDeviceQueue(m_pContext->device, indices.presentFamily.value(), 0, &presentQueue);
}