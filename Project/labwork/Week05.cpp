#include <set>

#include "vulkanbase/VulkanBase.h"

void VulkanBase::pickPhysicalDevice()
{
	uint32_t deviceCount = 0;

	vkEnumeratePhysicalDevices(m_pContext->instance, &deviceCount, nullptr);

	if (deviceCount == 0) 
	{
		throw std::runtime_error("failed to find GPUs with Vulkan support!");
	}

	std::vector<VkPhysicalDevice> devices{ deviceCount };
	vkEnumeratePhysicalDevices(m_pContext->instance, &deviceCount, devices.data());

	if (deviceCount == 0) 
	{
		throw std::runtime_error("failed to find GPUs with Vulkan support!");
	}

	for (const auto& device : devices) 
	{
		if (isDeviceSuitable(device)) 
		{ 
			m_pContext->physicalDevice = device;
			break;
		}
	}

	if (m_pContext->physicalDevice == VK_NULL_HANDLE) 
	{
		throw std::runtime_error("failed to find a suitable GPU!");
	}
}

bool VulkanBase::isDeviceSuitable(VkPhysicalDevice device)
{
	auto surface = m_pContext->surface;
	QueueFamilyIndices indices = QueueFamilyIndices::FindQueueFamilies(device, surface);
	bool extensionsSupported = checkDeviceExtensionSupport(device);
	return indices.isComplete() && extensionsSupported;

}

void VulkanBase::createLogicalDevice()
{
	auto physicalDevice = m_pContext->physicalDevice;
	auto surface = m_pContext->surface;

	QueueFamilyIndices indices = QueueFamilyIndices::FindQueueFamilies(physicalDevice, surface);

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

	VkPhysicalDeviceFeatures deviceFeatures{};

	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos = queueCreateInfos.data();

	createInfo.pEnabledFeatures = &deviceFeatures;

	createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	createInfo.ppEnabledExtensionNames = deviceExtensions.data();

	if (enableValidationLayers) 
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else 
	{
		createInfo.enabledLayerCount = 0;
	}

	if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &m_pContext->device) != VK_SUCCESS) 
	{
		throw std::runtime_error("failed to create logical device!");
	}



	vkGetDeviceQueue(m_pContext->device, indices.graphicsFamily.value(), 0, &graphicsQueue);
	vkGetDeviceQueue(m_pContext->device, indices.presentFamily.value(), 0, &presentQueue);
}