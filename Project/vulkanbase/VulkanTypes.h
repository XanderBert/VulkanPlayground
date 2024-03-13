#pragma once
#include <vulkan/vulkan.h>
#include <vulkanbase/VulkanUtil.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <iostream>
#include <vector>


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
	const uint32_t WIDTH = 800;
	const uint32_t HEIGHT = 600;


	void InitWindow()
	{
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
		m_pWindow = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
	}
};



//TODO Make this class static?
class VulkanContext 
{
public:
	VulkanContext() = default;
	~VulkanContext() = default;
	VulkanContext(const VulkanContext&) = delete;
	VulkanContext& operator=(const VulkanContext&) = delete;
	VulkanContext(VulkanContext&&) = delete;
	VulkanContext& operator=(VulkanContext&&) = delete;

	//Week 06
	void CreateSurface()
	{
		if (glfwCreateWindowSurface(instance, window.Ptr(), nullptr, &surface) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create window surface!");
		}
	}

    VkInstance instance{};
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;
    VkSurfaceKHR surface{};
    VkSwapchainKHR swapchain{};
	Window window{};


    void CleanUp() const
    {
		vkDestroySwapchainKHR(device, swapchain, nullptr);
		vkDestroyDevice(device, nullptr);
		vkDestroySurfaceKHR(instance, surface, nullptr);
		vkDestroyInstance(instance, nullptr);
	}
};