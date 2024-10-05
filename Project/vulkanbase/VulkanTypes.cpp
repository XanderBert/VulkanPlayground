#include "VulkanTypes.h"

Window::Window(const std::string& windowName,int width, int height)
    : m_WindowName(windowName)
    , m_Width(width)
    , m_Height(height)
{
    InitWindow();
}


Window::~Window()
{
    glfwDestroyWindow(m_pWindow);
    glfwTerminate();
}

void Window::GetSize(int &width, int &height) const {
    glfwGetFramebufferSize(m_pWindow, &width, &height);
}

bool Window::ShouldClose() const {
    return glfwWindowShouldClose(m_pWindow);
}

bool Window::IsMinimized() const {
    int width, height;
    glfwGetFramebufferSize(m_pWindow, &width, &height);
    return width == 0 || height == 0;
}

void Window::PollEvents()
{
    glfwPollEvents();
}

void Window::InitWindow()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    m_pWindow = glfwCreateWindow(m_Width, m_Height, "Vulkan Shader Editor", nullptr, nullptr);
    glfwMakeContextCurrent(m_pWindow);
}

VulkanContext::VulkanContext(const std::string &windowName, int width, int height)
: window(windowName, width, height)
{}


std::filesystem::path VulkanContext::GetAssetPath()
{
    return std::filesystem::current_path()/ "Assets";
}

void VulkanContext::CleanUp() const
{
    vkDestroyCommandPool(device, commandPool, nullptr);
    vkDestroyDevice(device, nullptr);
    vkDestroyInstance(instance, nullptr);
}