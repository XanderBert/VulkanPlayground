#include <set>
#include "Core/DepthResource.h"
#include "Core/Descriptor.h"
#include "Core/SwapChain.h"
#include "shaders/Logic/Shader.h"
#include "vulkanbase/VulkanBase.h"




VkDebugUtilsMessengerCreateInfoEXT VulkanBase::setupDebugMessenger()
{
	VkDebugUtilsMessengerCreateInfoEXT createInfo{};

	populateDebugMessengerCreateInfo(createInfo);
	VulkanCheck(tools::CreateDebugUtilsMessengerEXT(m_pContext->instance, &createInfo, nullptr, &debugMessenger), "failed to set up debug messenger!");

	return createInfo;
}

void VulkanBase::createSyncObjects()
{
	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	if (vkCreateSemaphore(m_pContext->device, &semaphoreInfo, nullptr, &imageAvailableSemaphore) != VK_SUCCESS ||
		vkCreateSemaphore(m_pContext->device, &semaphoreInfo, nullptr, &renderFinishedSemaphore) != VK_SUCCESS ||
		vkCreateFence(m_pContext->device, &fenceInfo, nullptr, &inFlightFence) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create synchronization objects for a frame!");
	}

}


void VulkanBase::drawFrame()
{
	VkDevice device = m_pContext->device;
	vkWaitForFences(device, 1, &inFlightFence, VK_TRUE, UINT64_MAX);

    //TODO: This check should only happen on events / not in the hot code path
    ShaderManager::ReloadNeededShaders(m_pContext);

	uint32_t imageIndex;
	const VkResult nextImageResult = vkAcquireNextImageKHR(m_pContext->device, SwapChain::GetSwapChain(), UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);
	if (nextImageResult == VK_ERROR_OUT_OF_DATE_KHR)
	{
		SwapChain::SetNeedsRecreation();
		return;
	}
	else if (nextImageResult != VK_SUCCESS && nextImageResult != VK_SUBOPTIMAL_KHR)
	{
		LogError("Failed to acquire swap chain image!");
	}
	SwapChain::SetImageIndex(imageIndex);


	Descriptor::DescriptorManager::ClearPools(m_pContext->device);
	vkResetFences(device, 1, &inFlightFence);

	CommandBufferManager::ResetCommandBuffer(commandBuffer);
	CommandBufferManager::BeginCommandBufferRecording(commandBuffer, false, false);

	drawFrame(imageIndex);

	CommandBufferManager::EndCommandBufferRecording(commandBuffer);


	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &imageAvailableSemaphore;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer.Handle;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &renderFinishedSemaphore;

	CommandBufferManager::SubmitCommandBuffer(m_pContext, commandBuffer, &submitInfo, inFlightFence);


	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &renderFinishedSemaphore;
	const VkSwapchainKHR swapChains[] = { SwapChain::GetSwapChain() };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;




	const VkResult presentResult = vkQueuePresentKHR(presentQueue, &presentInfo);
	if (presentResult == VK_ERROR_OUT_OF_DATE_KHR || presentResult == VK_SUBOPTIMAL_KHR)
	{
		SwapChain::SetNeedsRecreation();
	}
	else if (presentResult != VK_SUCCESS)
	{
		LogError("Failed to present swap chain image! to queue");
	}
}

VkBool32 VulkanBase::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT, const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *)
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


bool VulkanBase::checkDeviceExtensionSupport(VkPhysicalDevice device)
{
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

	for (const auto& extension : availableExtensions)
	{
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}


void VulkanBase::createInstance()
{
	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Vulkan Playground";
	appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 1);
	appInfo.apiVersion = VK_API_VERSION_1_3;

	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	auto extensions = getRequiredExtensions();
	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();

#ifndef NDEBUG
	LogAssert(CheckValidationLayerSupport(m_ValidationLayers), "Validation layers requested, but not available!", false);

	createInfo.enabledLayerCount = static_cast<uint32_t>(m_ValidationLayers.size());
	createInfo.ppEnabledLayerNames = m_ValidationLayers.data();


	std::vector<VkValidationFeatureEnableEXT> enabledValidationFeatures;
	enabledValidationFeatures.reserve(1);

	//enabledValidationFeatures.push_back(VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT);
	//enabledValidationFeatures.push_back(VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT);
	//enabledValidationFeatures.push_back(VK_VALIDATION_FEATURE_ENABLE_DEBUG_PRINTF_EXT);
	//enabledValidationFeatures.push_back(VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_RESERVE_BINDING_SLOT_EXT);
	enabledValidationFeatures.push_back(VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT);


	VkValidationFeaturesEXT validationFeatures{};
	validationFeatures.sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
	validationFeatures.enabledValidationFeatureCount = static_cast<uint32_t>(enabledValidationFeatures.size());
	validationFeatures.pEnabledValidationFeatures = enabledValidationFeatures.data();
	validationFeatures.disabledValidationFeatureCount = 0;

	createInfo.pNext = (VkValidationFeaturesEXT*)&validationFeatures;
#endif



	VulkanCheck(vkCreateInstance(&createInfo, nullptr, &m_pContext->instance), "Failed to create instance!");



#ifndef NDEBUG
	setupDebugMessenger();
#endif
}

std::vector<const char*> VulkanBase::getRequiredExtensions()
{
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

#ifndef NDEBUG
		extensions.push_back(VK_EXT_LAYER_SETTINGS_EXTENSION_NAME);
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

	return extensions;
}


void VulkanBase::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
	createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = debugCallback;
}

bool VulkanBase::CheckValidationLayerSupport(const std::vector<const char *> &validationLayers)
{
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : validationLayers)
	{
		bool layerFound = false;

		for (const auto& layerProperties : availableLayers)
		{
			if (strcmp(layerName, layerProperties.layerName) == 0)
			{
				layerFound = true;
				break;
			}
		}

		if (!layerFound) {
			return false;
		}
	}

	return true;
}
