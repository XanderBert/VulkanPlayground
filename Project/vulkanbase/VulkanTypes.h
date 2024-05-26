#pragma once

#include <vulkan/vulkan.h>
#include <vulkanbase/VulkanUtil.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <__msvc_filebuf.hpp>
#include <filesystem>
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

	void GetSize(int& width, int& height) const
	{
		glfwGetFramebufferSize(m_pWindow, &width, &height);
	}

	bool ShouldClose() const
	{
		return glfwWindowShouldClose(m_pWindow);
	}

	void PollEvents() const
	{
		glfwPollEvents();
	}

	bool IsMinimized() const
	{
		int width, height;
		glfwGetFramebufferSize(m_pWindow, &width, &height);
		return width == 0 || height == 0;
	}

	GLFWwindow* Ptr() const { return m_pWindow; }

private:
	GLFWwindow* m_pWindow;
	const uint32_t WIDTH = 1280;
	const uint32_t HEIGHT = 720;


	inline void InitWindow()
	{
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
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


    static std::filesystem::path GetAssetPath(const std::string& path)
    {
        return std::filesystem::current_path().parent_path().parent_path() / "Assets" / path;
    }


    inline void CleanUp() const
    {
		vkDestroyCommandPool(device, commandPool, nullptr);
		vkDestroyDevice(device, nullptr);
		vkDestroyInstance(instance, nullptr);
	}
};