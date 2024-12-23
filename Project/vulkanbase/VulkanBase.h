#pragma once

#if defined(_WIN32)
#define VK_USE_PLATFORM_WIN32_KHR
#define WIN32_LEAN_AND_MEAN
#elif defined(__linux__)
#define VK_USE_PLATFORM_WAYLAND_KHR
#endif


#include <vulkan/vulkan.h>

#include "Core/CommandBuffer.h"
#include "shaders/Logic/ShaderFileWatcher.h"


class VulkanContext;
struct ImGui_ImplVulkan_InitInfo;

const std::vector<const char*> deviceExtensions = 
{
	VK_KHR_SWAPCHAIN_EXTENSION_NAME,

	VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
	VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME,
	VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME,
};



class VulkanBase
{
public:
	void run();

private:
	void initVulkan();
	void mainLoop();

    void cleanup() const;

    VulkanContext* m_pContext{};
	ShaderFileWatcher shaderFileWatcher{};
    CommandBuffer commandBuffer{};
	PFN_vkCmdBeginRenderingKHR vkCmdBeginRenderingKHR{ VK_NULL_HANDLE };
	PFN_vkCmdEndRenderingKHR vkCmdEndRenderingKHR{ VK_NULL_HANDLE };


	const std::vector<const char*> m_ValidationLayers = { "VK_LAYER_KHRONOS_validation" };


	VkQueue presentQueue;
	VkDebugUtilsMessengerEXT debugMessenger;
	VkSemaphore imageAvailableSemaphore;
	VkSemaphore renderFinishedSemaphore;
	VkFence inFlightFence;


	void drawFrame(uint32_t imageIndex) const;


    void pickPhysicalDevice();
    static int rateDeviceSuitability(VkPhysicalDevice device);
    static bool isDeviceSuitable(VkPhysicalDevice device);
	void createLogicalDevice();
	VkDebugUtilsMessengerCreateInfoEXT setupDebugMessenger();
    static bool checkDeviceExtensionSupport(VkPhysicalDevice device);
	void createInstance();
	std::vector<const char*> getRequiredExtensions();
	static void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
	static bool CheckValidationLayerSupport(const std::vector<const char*>& validationLayers);
	void createSyncObjects();
	void drawFrame();

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void*);
};