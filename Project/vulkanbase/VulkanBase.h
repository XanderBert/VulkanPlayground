#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#include "VulkanUtil.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <cstdint>

#include "Mesh/Vertex.h"

#include "shaders/Logic/ShaderFileWatcher.h"
#include "Core/CommandBuffer.h"
#include "Core/CommandPool.h"


#include "shaders/Logic/Shader.h"
#include "Patterns/ServiceLocator.h"
#include "VulkanTypes.h"
#include "Scene/Scene.h"
#include "Core/ImGuiWrapper.h"
#include "Core/DepthResource.h"
#include "Core/Logger.h"
#include "Input/Input.h"

#include <thread>
#include "Core/SwapChain.h"
#include "Mesh/MaterialManager.h"
#include "imgui_impl_vulkan.h"

#include "Scene/SceneManager.h"

#include "Core/GlobalDescriptor.h"
#include "shaders/Logic/ShaderEditor.h"


struct ImGui_ImplVulkan_InitInfo;
const std::vector validationLayers = { "VK_LAYER_KHRONOS_validation" };
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
	void run()
	{
		ServiceConfigurator::Configure();
		initVulkan();
		ImGuiWrapper::Initialize(m_pContext->graphicsQueue);
        ShaderEditor::Init();
	    SceneManager::AddScene(std::make_unique<Scene>(m_pContext));
	    Input::SetupInput(m_pContext->window.Ptr());
		MaterialManager::CreatePipelines();
		mainLoop();
		cleanup();
	}

private:
	void initVulkan()
	{
		m_pContext = ServiceLocator::GetService<VulkanContext>();
 		createInstance();
		setupDebugMessenger();

		SwapChain::CreateSurface(m_pContext);
		pickPhysicalDevice();
		createLogicalDevice();
		SwapChain::Init(m_pContext);
		DepthResource::Init(m_pContext);
        Camera::Init();

		// Since we use an extension, we need to expliclity load the function pointers for extension related Vulkan commands
		vkCmdBeginRenderingKHR = reinterpret_cast<PFN_vkCmdBeginRenderingKHR>(vkGetDeviceProcAddr(m_pContext->device, "vkCmdBeginRenderingKHR"));
		vkCmdEndRenderingKHR = reinterpret_cast<PFN_vkCmdEndRenderingKHR>(vkGetDeviceProcAddr(m_pContext->device, "vkCmdEndRenderingKHR"));

		CommandPool::CreateCommandPool(m_pContext);
		CommandBufferManager::CreateCommandBuffer(m_pContext, commandBuffer);
		Descriptor::DescriptorManager::Init(m_pContext);
		createSyncObjects();
	    ShaderManager::Setup();

	    LogInfo("Vulkan Initialized");
	}

	void mainLoop()
	{
		const Window& window = m_pContext->window;


		while (!window.ShouldClose()) 
		{
			window.PollEvents();

			if(window.IsMinimized())
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
				continue;
			}

			SwapChain::RecreateIfNeeded(m_pContext);

			ImGuiWrapper::NewFrame();
			drawFrame();
			ImGuiWrapper::EndFrame();
		}

		vkDeviceWaitIdle(m_pContext->device);
	}

	void cleanup() const
	{
		const VkDevice device = m_pContext->device;

		vkDestroySemaphore(device, renderFinishedSemaphore, nullptr);
		vkDestroySemaphore(device, imageAvailableSemaphore, nullptr);
		vkDestroyFence(device, inFlightFence, nullptr);

		DepthResource::Cleanup(m_pContext);

		if (enableValidationLayers) 
		{
			tools::DestroyDebugUtilsMessengerEXT(m_pContext->instance, debugMessenger, nullptr);
		}

		Descriptor::DescriptorManager::Cleanup(m_pContext->device);
		ShaderManager::Cleanup(m_pContext->device);
		MaterialManager::Cleanup();
		SceneManager::CleanUp();
	    ImGuiWrapper::Cleanup();
		SwapChain::Cleanup(m_pContext);
		m_pContext->CleanUp();
	}

	VulkanContext* m_pContext{};
	ShaderFileWatcher shaderFileWatcher{};
	CommandBuffer commandBuffer{};
	PFN_vkCmdBeginRenderingKHR vkCmdBeginRenderingKHR{ VK_NULL_HANDLE };
	PFN_vkCmdEndRenderingKHR vkCmdEndRenderingKHR{ VK_NULL_HANDLE };

	VkQueue presentQueue;
	VkDebugUtilsMessengerEXT debugMessenger;
	VkSemaphore imageAvailableSemaphore;
	VkSemaphore renderFinishedSemaphore;
	VkFence inFlightFence;


	void drawFrame(uint32_t imageIndex) const
	{
        VkExtent2D& swapChainExtent = SwapChain::Extends();

		VkRenderingAttachmentInfoKHR colorAttachmentInfo{};
		colorAttachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
		colorAttachmentInfo.pNext = VK_NULL_HANDLE;
		colorAttachmentInfo.imageView = SwapChain::ImageViews()[imageIndex];
		colorAttachmentInfo.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		colorAttachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachmentInfo.clearValue = {{0.83f, 0.75f, 0.83f, 1.0f}};


		VkRenderingAttachmentInfoKHR depthAttachmentInfo{};
		depthAttachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
		depthAttachmentInfo.pNext = VK_NULL_HANDLE;
		depthAttachmentInfo.imageView = DepthResource::GetImageView();
		depthAttachmentInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		depthAttachmentInfo.resolveMode = VK_RESOLVE_MODE_NONE;
		depthAttachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		depthAttachmentInfo.clearValue = {{1.0f, 0.0f}};


	    VkRenderingAttachmentInfoKHR depthAttatchements[1] = { depthAttachmentInfo};
		VkRenderingInfoKHR renderInfo{};
		renderInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR;
		renderInfo.renderArea = { 0, 0, swapChainExtent };
		renderInfo.layerCount = 1;
		renderInfo.colorAttachmentCount = 1;
		renderInfo.pColorAttachments = &colorAttachmentInfo;
		renderInfo.pDepthAttachment = depthAttatchements;
		renderInfo.pStencilAttachment = VK_NULL_HANDLE;


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


		vkCmdBeginRenderingKHR(commandBuffer.Handle, &renderInfo);
        SceneManager::Render(commandBuffer.Handle);
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer.Handle);
		vkCmdEndRenderingKHR(commandBuffer.Handle);
	}



	void pickPhysicalDevice();
	bool isDeviceSuitable(VkPhysicalDevice device);
	void createLogicalDevice();
	void setupDebugMessenger();
    static bool checkDeviceExtensionSupport(VkPhysicalDevice device);
	void createInstance();
	std::vector<const char*> getRequiredExtensions();
	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
	void createSyncObjects();
	void drawFrame();

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void*)
	{
		VulkanLogger::LogLevel logLevel = VulkanLogger::LogLevel::LOGERROR;

		switch (messageSeverity)
		{
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
			logLevel = VulkanLogger::LogLevel::INFO;
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
			logLevel = VulkanLogger::LogLevel::WARNING;
			break;
		default:
			break;
		}

		LogMessage(logLevel, pCallbackData->pMessage);
		return VK_FALSE;
	}
};