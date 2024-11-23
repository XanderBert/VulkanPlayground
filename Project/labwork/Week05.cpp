#include <imgui_impl_vulkan.h>
#include <set>
#include "Core/SwapChain.h"

#include "Core/ColorAttachment.h"
#include "Core/DepthResource.h"
#include "Core/GBuffer.h"
#include "Core/GlobalDescriptor.h"
#include "Mesh/MaterialManager.h"
#include "Scene/SceneManager.h"
#include "vulkanbase/VulkanBase.h"
#include "vulkanbase/VulkanTypes.h"

void VulkanBase::drawFrame(uint32_t imageIndex) const
{
	//TODO: Move to swapchain
	tools::InsertImageMemoryBarrier(
			commandBuffer.Handle,
			SwapChain::Image(static_cast<uint8_t>(imageIndex)),
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });

	VkExtent2D& swapChainExtent = SwapChain::Extends();
	VulkanWindow::SetViewportCmd(commandBuffer.Handle);

    // ======================= Depth-Only Pass ============================
	GBuffer::GetDepthAttachment()->ResetImageLayout();
	GBuffer::GetDepthAttachment()->TransitionToDepthResource(commandBuffer.Handle);

	GBuffer::GetColorAttachmentNormal()->ResetImageLayout();
	GBuffer::GetColorAttachmentNormal()->TransitionToWrite(commandBuffer.Handle);





	// Create rendering info
	VkRenderingInfoKHR depthRenderInfo{};
	depthRenderInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR;
	depthRenderInfo.renderArea = {0, 0, swapChainExtent};
	depthRenderInfo.layerCount = 1;
	depthRenderInfo.colorAttachmentCount = 1;
	depthRenderInfo.pColorAttachments = GBuffer::GetColorAttachmentNormal()->GetRenderingAttachmentInfo();
	depthRenderInfo.pDepthAttachment = GBuffer::GetDepthAttachment()->GetRenderingAttachmentInfo();

	vkCmdBeginRenderingKHR(commandBuffer.Handle, &depthRenderInfo);
    SceneManager::RenderDepth(commandBuffer.Handle);
	vkCmdEndRenderingKHR(commandBuffer.Handle);


    // ======================= Downsample Depth Pass ============================

    // Transition depth buffer to be used in the compute pass
	GBuffer::GetDepthAttachment()->TransitionToGeneralResource(commandBuffer.Handle);
	GBuffer::GetColorAttachmentNormal()->TransitionToGeneralResource(commandBuffer.Handle);

	// Execute the compute pass
	SceneManager::ExecuteComputePass(commandBuffer.Handle);

	//Transition depth buffer to be used in the color pass
	GBuffer::GetColorAttachmentNormal()->TransitionToRead(commandBuffer.Handle);
	GBuffer::GetDepthAttachment()->TransitionToShaderRead(commandBuffer.Handle);












	// ======================= Final Color Rendering Pass ============================
    VkRenderingInfoKHR renderInfo{};
    renderInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR;
    renderInfo.renderArea = { 0, 0, swapChainExtent };
    renderInfo.layerCount = 1;
    renderInfo.colorAttachmentCount = 1;
    renderInfo.pColorAttachments = GBuffer::GetAlbedoAttachment()->GetRenderingAttachmentInfo();
    renderInfo.pDepthAttachment = GBuffer::GetDepthAttachment()->GetRenderingAttachmentInfo();
    renderInfo.pStencilAttachment = VK_NULL_HANDLE;




	GBuffer::GetAlbedoAttachment()->ResetImageLayout();
	GBuffer::GetAlbedoAttachment()->TransitionToWrite(commandBuffer.Handle);



    vkCmdBeginRenderingKHR(commandBuffer.Handle, &renderInfo);
    SceneManager::Render(commandBuffer.Handle);
    vkCmdEndRenderingKHR(commandBuffer.Handle);

	//Here we write to albedo
	//Albedo should transition to read
	GBuffer::GetAlbedoAttachment()->TransitionToRead(commandBuffer.Handle);






	//TODO: Move to swapchain
	VkRenderingAttachmentInfoKHR colorAttachmentInfo{};
	colorAttachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
	colorAttachmentInfo.pNext = VK_NULL_HANDLE;
	colorAttachmentInfo.imageView = SwapChain::ImageViews()[imageIndex];
	colorAttachmentInfo.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	colorAttachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachmentInfo.clearValue = {{0.83f, 0.75f, 0.83f, 1.0f}};

	VkRenderingInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR;
	presentInfo.renderArea = { 0, 0, swapChainExtent };
	presentInfo.layerCount = 1;
	presentInfo.colorAttachmentCount = 1;
	presentInfo.pColorAttachments = &colorAttachmentInfo;
	presentInfo.pDepthAttachment = GBuffer::GetDepthAttachment()->GetRenderingAttachmentInfo();
	presentInfo.pStencilAttachment = VK_NULL_HANDLE;



	vkCmdBeginRenderingKHR(commandBuffer.Handle, &presentInfo);
	SceneManager::RenderPresent(commandBuffer.Handle);
	//Take Albedo and SSAO as input -> Render to SwapChain
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer.Handle);
	vkCmdEndRenderingKHR(commandBuffer.Handle);


	//TODO: Move to swapchain
    tools::InsertImageMemoryBarrier(
        commandBuffer.Handle,
        SwapChain::Image(static_cast<uint8_t>(imageIndex)),
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });
}

void VulkanBase::pickPhysicalDevice()
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(m_pContext->instance, &deviceCount, nullptr);
    LogAssert(deviceCount > 0, "failed to find GPUs with Vulkan support!", true);

    std::vector<VkPhysicalDevice> devices{ deviceCount };
    vkEnumeratePhysicalDevices(m_pContext->instance, &deviceCount, devices.data());
    LogAssert(deviceCount > 0, "failed to find GPUs with Vulkan support!", true);

    VkPhysicalDevice bestDevice = VK_NULL_HANDLE;
    int bestScore = -1;

    for (const auto& device : devices)
    {
        int score = rateDeviceSuitability(device);
        if (score > bestScore && isDeviceSuitable(device))
        {
            bestDevice = device;
            bestScore = score;
        }
    }

    m_pContext->physicalDevice = bestDevice;
    LogAssert(m_pContext->physicalDevice != VK_NULL_HANDLE, "failed to find a suitable GPU!", true);
}

int VulkanBase::rateDeviceSuitability(VkPhysicalDevice device)
{
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    int score{};


    if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)  score += 1000;

    // Maximum possible texture dimensions affect overall graphics quality
    score += deviceProperties.limits.maxImageDimension2D;

    if (!deviceFeatures.samplerAnisotropy)  return 0;


    //TODO: add other criteria, like memory, queues, etc.
    return score;
}


bool VulkanBase::isDeviceSuitable(VkPhysicalDevice device)
{
	VkPhysicalDeviceFeatures supportedFeatures;
	vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

	const QueueFamilyIndices indices = QueueFamilyIndices::FindQueueFamilies(device, SwapChain::GetSurface());

	const bool extensionsSupported = checkDeviceExtensionSupport(device);

	return indices.isComplete() && extensionsSupported && supportedFeatures.samplerAnisotropy;
}

void VulkanBase::createLogicalDevice()
{
	VkPhysicalDevice physicalDevice = m_pContext->physicalDevice;
	
	QueueFamilyIndices indices = QueueFamilyIndices::FindQueueFamilies(physicalDevice, SwapChain::GetSurface());

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

	float queuePriority = 1.0f;
	for (uint32_t queueFamily : uniqueQueueFamilies)
	{
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	//Set the device features
	VkPhysicalDeviceFeatures deviceFeatures{};
	deviceFeatures.samplerAnisotropy = VK_TRUE;
    deviceFeatures.fragmentStoresAndAtomics = VK_TRUE;
    deviceFeatures.vertexPipelineStoresAndAtomics = VK_TRUE;


	//Set the Dynamic Rendering Extension
	VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamicRenderingFeature{};
	dynamicRenderingFeature.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR;
	dynamicRenderingFeature.dynamicRendering = VK_TRUE;

    //VkPhysicalDeviceTimelineSemaphoreFeaturesKHR timelineSemaphoreFeatures{};
    //timelineSemaphoreFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES_KHR;
    //timelineSemaphoreFeatures.timelineSemaphore = VK_TRUE;
    //timelineSemaphoreFeatures.pNext = &dynamicRenderingFeature;

    //VkPhysicalDeviceBufferDeviceAddressFeatures bufferDeviceAddressFeatures{};
    //bufferDeviceAddressFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES;
    //bufferDeviceAddressFeatures.bufferDeviceAddress = VK_TRUE;
    //bufferDeviceAddressFeatures.pNext = &timelineSemaphoreFeatures;

	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.pEnabledFeatures = &deviceFeatures;
	createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	createInfo.ppEnabledExtensionNames = deviceExtensions.data();
	createInfo.pNext = &dynamicRenderingFeature;

	if (enableValidationLayers) 
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}

	createInfo.enabledLayerCount = 0;
	

	
	VulkanCheck(vkCreateDevice(physicalDevice, &createInfo, nullptr, &m_pContext->device), "failed to create logical device!");

	vkGetDeviceQueue(m_pContext->device, indices.graphicsFamily.value(), 0, &m_pContext->graphicsQueue);
	vkGetDeviceQueue(m_pContext->device, indices.presentFamily.value(), 0, &presentQueue);
}