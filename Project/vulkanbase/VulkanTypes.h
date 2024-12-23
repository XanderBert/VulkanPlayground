#pragma once


#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #define GLFW_EXPOSE_NATIVE_WIN32
    #include <windows.h>
#elif __linux__
    #define GLFW_EXPOSE_NATIVE_X11
    #include <X11/Xlib.h>
#endif

#include <vulkan/vulkan.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>


#include <filesystem>



class VulkanWindow
{
public:

    //Creates The window and initializes glfw
	explicit VulkanWindow(const std::string& windowName, int width = 1280, int height = 720);

    //Destroys the window and terminates glfw
	~VulkanWindow();

	VulkanWindow(const VulkanWindow&) = delete;
	VulkanWindow& operator=(const VulkanWindow&) = delete;
	VulkanWindow(VulkanWindow&&) = delete;
	VulkanWindow& operator=(VulkanWindow&&) = delete;

	void GetSize(int& width, int& height) const;

    [[nodiscard]] bool ShouldClose() const;

    [[nodiscard]] bool IsMinimized() const;

    [[nodiscard]] GLFWwindow* Get() const { return m_pWindow; }

    static void PollEvents();
	static void SetViewportCmd(VkCommandBuffer commandBuffer);
private:
	GLFWwindow* m_pWindow{};
    std::string m_WindowName;
	const int m_Width;
	const int m_Height;

    void InitWindow();
};


class VulkanContext 
{
public:
	explicit VulkanContext(const std::string& windowName, int width = 1280, int height = 720);

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
	VulkanWindow window;

    static std::filesystem::path GetAssetPath();
    void CleanUp() const;
};