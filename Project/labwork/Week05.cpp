#include <set>
#include "Core/SwapChain.h"
#include <imgui_impl_vulkan.h>

#include "Core/DepthResource.h"
#include "Scene/SceneManager.h"
#include "vulkanbase/VulkanBase.h"
#include "vulkanbase/VulkanTypes.h"

void VulkanBase::drawFrame(uint32_t imageIndex) const
{
    VkExtent2D& swapChainExtent = SwapChain::Extends();
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



    // ======================= Depth-Only Pass ============================
    tools::InsertImageMemoryBarrier(
        commandBuffer.Handle,
        DepthResource::GetImage(),
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        VkImageSubresourceRange{ VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1 });




    VkRenderingAttachmentInfoKHR depthAttachmentInfo{};
    depthAttachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
    depthAttachmentInfo.pNext = VK_NULL_HANDLE;
    depthAttachmentInfo.imageView = DepthResource::GetImageView();
    depthAttachmentInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depthAttachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depthAttachmentInfo.clearValue = {{1.0f, 0.0f}};  // Clear depth to 1 (far)

    // Create rendering info (no color attachment, only depth)
    VkRenderingInfoKHR depthRenderInfo{};
    depthRenderInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR;
    depthRenderInfo.renderArea = {0, 0, swapChainExtent};
    depthRenderInfo.layerCount = 1;
    depthRenderInfo.colorAttachmentCount = 0;  // No color attachment
    depthRenderInfo.pDepthAttachment = &depthAttachmentInfo;

    vkCmdBeginRenderingKHR(commandBuffer.Handle, &depthRenderInfo);
    SceneManager::RenderDepth(commandBuffer.Handle);
    vkCmdEndRenderingKHR(commandBuffer.Handle);


    // Transition depth buffer to be used in the color pass
    tools::InsertImageMemoryBarrier(
        commandBuffer.Handle,
        DepthResource::GetImage(),
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        //VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,  // Make it readable for color pass
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,  // Make it readable for color pass
        VkImageSubresourceRange{VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1});


    // ======================= Compute SSAO Pass ============================




    // ======================= Final Color Rendering Pass ============================
    VkRenderingAttachmentInfoKHR colorAttachmentInfo{};
    colorAttachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
    colorAttachmentInfo.pNext = VK_NULL_HANDLE;
    colorAttachmentInfo.imageView = SwapChain::ImageViews()[imageIndex];
    colorAttachmentInfo.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorAttachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachmentInfo.clearValue = {{0.83f, 0.75f, 0.83f, 1.0f}};


    depthAttachmentInfo.imageView = DepthResource::GetImageView();
    //depthAttachmentInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    depthAttachmentInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;


    VkRenderingInfoKHR renderInfo{};
    renderInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR;
    renderInfo.renderArea = { 0, 0, swapChainExtent };
    renderInfo.layerCount = 1;
    renderInfo.colorAttachmentCount = 1;
    renderInfo.pColorAttachments = &colorAttachmentInfo;
    renderInfo.pDepthAttachment = &depthAttachmentInfo;
    renderInfo.pStencilAttachment = VK_NULL_HANDLE;





    tools::InsertImageMemoryBarrier(
            commandBuffer.Handle,
            SwapChain::Image(static_cast<uint8_t>(imageIndex)),
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });



    vkCmdBeginRenderingKHR(commandBuffer.Handle, &renderInfo);
    SceneManager::Render(commandBuffer.Handle);
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer.Handle);
    vkCmdEndRenderingKHR(commandBuffer.Handle);


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


	for (const auto& device : devices) 
	{
		if (isDeviceSuitable(device)) 
		{ 
			m_pContext->physicalDevice = device;
			break;
		}
	}

	LogAssert(m_pContext->physicalDevice != VK_NULL_HANDLE, "failed to find a suitable GPU!", true);
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

	//Set the Dynamic Rendering Extension
	VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamicRenderingFeature{};
	dynamicRenderingFeature.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR;
	dynamicRenderingFeature.dynamicRendering = VK_TRUE;


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