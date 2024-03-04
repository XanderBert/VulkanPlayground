#pragma once
struct QueueFamilyIndices
{
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	static QueueFamilyIndices FindQueueFamilies(const VkPhysicalDevice& physicalDevice, const VkSurfaceKHR& surface) 
	{
		QueueFamilyIndices indices;

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

		int i = 0;
		for (const auto& queueFamily : queueFamilies)
		{
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)indices.graphicsFamily = i;

			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);

			if (presentSupport) indices.presentFamily = i;
			if (indices.isComplete()) break;

			++i;
		}

		return indices;
	}

	bool isComplete() const
	{
		return graphicsFamily.has_value() && presentFamily.has_value();
	}
};

