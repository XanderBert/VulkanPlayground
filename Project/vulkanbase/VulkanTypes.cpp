#include "VulkanTypes.h"

#include "Core/SwapChain.h"

VulkanWindow::VulkanWindow(const std::string &windowName, int width, int height) :
	m_WindowName(windowName), m_Width(width), m_Height(height)
{
	InitWindow();
}


VulkanWindow::~VulkanWindow()
{
	glfwDestroyWindow(m_pWindow);
	glfwTerminate();
}

void VulkanWindow::GetSize(int &width, int &height) const
{
	glfwGetFramebufferSize(m_pWindow, &width, &height);
}

bool VulkanWindow::ShouldClose() const
{
	return glfwWindowShouldClose(m_pWindow);
}

bool VulkanWindow::IsMinimized() const
{
	int width, height;
	glfwGetFramebufferSize(m_pWindow, &width, &height);
	return width == 0 || height == 0;
}

void VulkanWindow::PollEvents()
{
	glfwPollEvents();
}

void VulkanWindow::SetViewportCmd(VkCommandBuffer commandBuffer)
{
	VkExtent2D &swapChainExtent = SwapChain::Extends();

	//Set the viewport
	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)swapChainExtent.width;
	viewport.height = (float)swapChainExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

	//Set the scissor
	VkRect2D scissor{};
	scissor.offset = {0, 0};
	scissor.extent = swapChainExtent;
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}

void VulkanWindow::InitWindow()
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
	m_pWindow = glfwCreateWindow(m_Width, m_Height, "Vulkan Shader Editor", nullptr, nullptr);
	glfwMakeContextCurrent(m_pWindow);
}

VulkanContext::VulkanContext(const std::string &windowName, int width, int height):
	window(windowName, width, height)
{}


std::filesystem::path VulkanContext::GetAssetPath()
{
	return std::filesystem::current_path() / "Assets";
}

void VulkanContext::CleanUp() const
{
	vkDestroyCommandPool(device, commandPool, nullptr);
	vkDestroyDevice(device, nullptr);
	vkDestroyInstance(instance, nullptr);
}
