#include "VulkanUtil.h"

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
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

