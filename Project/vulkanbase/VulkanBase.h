#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#include "VulkanUtil.h"

#include <iostream>
#include <vector>
#include <cstdint>
#include <optional>


#include "Mesh/Vertex.h"

#include "../shaders/ShaderFileWatcher.h"
#include "Core/CommandBuffer.h"
#include "Core/CommandPool.h"
#include "Core/GraphicsPipeline.h"
#include "Core/QueueFamilyIndices.h"


#include "../Patterns/ServiceLocator.h"
#include "VulkanTypes.h"
#include "Scene/Scene.h"
#include "../Core/ImGuiWrapper.h"

struct ImGui_ImplVulkan_InitInfo;
const std::vector validationLayers = { "VK_LAYER_KHRONOS_validation" };
//const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME, VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME};
const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME};



struct SwapChainSupportDetails
{
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};


class VulkanBase
{
public:
	void run()
	{
		ServiceConfigurator::Configure();
		initVulkan();
		ImGuiWrapper::Initialize(graphicsQueue, swapChainImages.size(), m_pGraphicsPipeline.GetRenderPass());
		m_pScene = std::make_unique<Scene>();
		mainLoop();
		cleanup();
	}

private:
	void initVulkan()
	{
		// week 06
		m_pContext = ServiceLocator::GetService<VulkanContext>();
 		createInstance();
		setupDebugMessenger();
		m_pContext->CreateSurface();

		// week 05
		pickPhysicalDevice();
		createLogicalDevice();

		// week 04 
		createSwapChain();
		createImageViews();

		// week 03
		createGraphicsPipeline();
		createFrameBuffers();
	
		CommandPool::CreateCommandPool(m_pContext, commandPool);
		CommandBufferManager::CreateCommandBuffer(m_pContext->device, commandPool, commandBuffer);

		// week 06
		createSyncObjects();
	}



	void mainLoop()
	{

		while (!glfwWindowShouldClose(m_pContext->window.Ptr())) 
		{
			glfwPollEvents();			
			ImGuiWrapper::NewFrame();
			drawFrame();
		}

		vkDeviceWaitIdle(m_pContext->device);
	}

	void cleanup() const
	{
		ImGuiWrapper::Cleanup();

		const VkDevice device = m_pContext->device;

		vkDestroySemaphore(device, renderFinishedSemaphore, nullptr);
		vkDestroySemaphore(device, imageAvailableSemaphore, nullptr);
		vkDestroyFence(device, inFlightFence, nullptr);

		vkDestroyCommandPool(device, commandPool, nullptr);
		for (const auto framebuffer : swapChainFramebuffers) 
		{
			vkDestroyFramebuffer(device, framebuffer, nullptr);
		}

		m_pGraphicsPipeline.Cleanup(device);


		for (const auto imageView : swapChainImageViews) 
		{
			vkDestroyImageView(device, imageView, nullptr);
		}

		if (enableValidationLayers) 
		{
			DestroyDebugUtilsMessengerEXT(m_pContext->instance, debugMessenger, nullptr);
		}

		m_pScene->CleanUp();
		m_pContext->CleanUp();
	}


	VulkanContext* m_pContext{};
	std::unique_ptr<Scene> m_pScene;

	//TODO : Move to a separate class (Service?)
	ShaderFileWatcher shaderFileWatcher{};



	VkCommandPool commandPool{};
	CommandBuffer commandBuffer{};


	void drawFrame(uint32_t imageIndex) const
	{
		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = m_pGraphicsPipeline.GetRenderPass();
		renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = swapChainExtent;

		constexpr VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;

		//Begin Render Pass
		vkCmdBeginRenderPass(commandBuffer.Handle, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		//Bin the Pipeline
		vkCmdBindPipeline(commandBuffer.Handle, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pGraphicsPipeline.GetPipeline());


		//Set the viewport
		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)swapChainExtent.width;
		viewport.height = (float)swapChainExtent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(commandBuffer.Handle, 0, 1, &viewport);


		//Set the scissor
		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = swapChainExtent;
		vkCmdSetScissor(commandBuffer.Handle, 0, 1, &scissor);

		//Render The actual scene
		m_pScene->Render(commandBuffer.Handle);
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer.Handle);


		//End the render pass
		vkCmdEndRenderPass(commandBuffer.Handle);
	}

	void createGraphicsPipeline();
	// Week 03
	GraphicsPipeline m_pGraphicsPipeline{};
	// Renderpass concept
	// Graphics pipeline


	std::vector<VkFramebuffer> swapChainFramebuffers;

	void createFrameBuffers();


	// Week 04
	// Swap chain and image view support

	std::vector<VkImage> swapChainImages;
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;

	std::vector<VkImageView> swapChainImageViews;

	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	void createSwapChain();
	void createImageViews();

	// Week 05 
	// Logical and physical device
	VkQueue graphicsQueue;
	VkQueue presentQueue;
	
	void pickPhysicalDevice();

	bool isDeviceSuitable(VkPhysicalDevice device);
	void createLogicalDevice();

	// Week 06
	// Main initialization
	VkDebugUtilsMessengerEXT debugMessenger;
	VkSemaphore imageAvailableSemaphore;
	VkSemaphore renderFinishedSemaphore;
	VkFence inFlightFence;

	void setupDebugMessenger();
	bool checkDeviceExtensionSupport(VkPhysicalDevice device);
	void createInstance();
	std::vector<const char*> getRequiredExtensions();
	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

	void createSyncObjects();
	void drawFrame();

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
	{
		std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
		return VK_FALSE;
	}
};