#pragma once

#include <vulkan/vulkan.h>
#include <vulkanbase/VulkanUtil.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <iostream>
#include <vector>
#include "Core/Logger.h"


class Window
{
public:
	Window()
	{
		InitWindow();
	}

	~Window()
	{
		glfwDestroyWindow(m_pWindow);
		glfwTerminate();
	}


	Window(const Window&) = delete;
	Window& operator=(const Window&) = delete;
	Window(Window&&) = delete;
	Window& operator=(Window&&) = delete;

	GLFWwindow* Ptr() const { return m_pWindow; }

private:
	GLFWwindow* m_pWindow;
	const uint32_t WIDTH = 1280;
	const uint32_t HEIGHT = 720;


	inline void InitWindow()
	{
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
		m_pWindow = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
		glfwMakeContextCurrent(m_pWindow);
	}
};


class VulkanContext 
{
public:
	VulkanContext() = default;
	~VulkanContext() = default;
	VulkanContext(const VulkanContext&) = delete;
	VulkanContext& operator=(const VulkanContext&) = delete;
	VulkanContext(VulkanContext&&) = delete;
	VulkanContext& operator=(VulkanContext&&) = delete;

	//TODO Store a CommandPool Manager here? (typically there would be 1 command pool for each thread)-> the same can be done for the graphics queue
	VkQueue graphicsQueue{};
	VkCommandPool commandPool{};
    VkInstance instance{};
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;
	Window window{};


    inline void CleanUp() const
    {
		vkDestroyCommandPool(device, commandPool, nullptr);
		vkDestroyDevice(device, nullptr);
		vkDestroyInstance(instance, nullptr);
	}
};