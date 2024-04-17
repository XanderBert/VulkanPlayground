#include "ImGuiWrapper.h"


#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"
#include "ImGuizmo.h"

#include "vulkanbase/VulkanTypes.h"
#include "Patterns/ServiceLocator.h"
#include "SwapChain.h"
#include "DepthResource.h"
#include "implot.h"

void ImGuiWrapper::Initialize(VkQueue graphicsQueue)
{
	const auto m_pContext = ServiceLocator::GetService<VulkanContext>();

	// Create Descriptor Pool TODO: Move to a separate function
	// The example only requires a single combined image sampler descriptor for the font image and only uses one descriptor set (for that)
	// If you wish to load e.g. additional textures you may need to alter pools sizes.
	{
		const VkDescriptorPoolSize pool_sizes[] =
		{
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1 },
		};

		VkDescriptorPoolCreateInfo pool_info = {};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		pool_info.maxSets = 1;
		pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
		pool_info.pPoolSizes = pool_sizes;
		auto err = vkCreateDescriptorPool(m_pContext->device, &pool_info, nullptr, &descriptorPool);
	}

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImPlot::CreateContext();

	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
	ImGui::StyleColorsDark();


	ImGui_ImplGlfw_InitForVulkan(m_pContext->window.Ptr(), true);
	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance = m_pContext->instance;
	init_info.PhysicalDevice = m_pContext->physicalDevice;
	init_info.Device = m_pContext->device;
	init_info.QueueFamily = QueueFamilyIndices::FindQueueFamilies(m_pContext->physicalDevice, SwapChain::GetSurface()).graphicsFamily.value();
	init_info.Queue = graphicsQueue;
	init_info.PipelineCache = VK_NULL_HANDLE;

	init_info.DescriptorPool = descriptorPool;
	init_info.Allocator = VK_NULL_HANDLE;
	init_info.MinImageCount = 2;
	init_info.ImageCount = SwapChain::ImageCount();
	init_info.CheckVkResultFn = nullptr;
	init_info.RenderPass = nullptr;
	init_info.UseDynamicRendering = true;

	init_info.PipelineRenderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
	init_info.PipelineRenderingCreateInfo.pColorAttachmentFormats = &SwapChain::Format();
	init_info.PipelineRenderingCreateInfo.colorAttachmentCount = 1;
	init_info.PipelineRenderingCreateInfo.depthAttachmentFormat = DepthResource::DepthResource::GetFormat();

	ImGui_ImplVulkan_Init(&init_info);
}

void ImGuiWrapper::Cleanup()
{
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImPlot::DestroyContext();
	ImGui::DestroyContext();

	vkDestroyDescriptorPool(ServiceLocator::GetService<VulkanContext>()->device, descriptorPool, nullptr);
}

void ImGuiWrapper::NewFrame()
{
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	//ImGuizmo::BeginFrame();
	ImGui::NewFrame();
	ImGuizmo::BeginFrame();
}

void ImGuiWrapper::EndFrame()
{
	ImGui::EndFrame();
	//ImGui::UpdatePlatformWindows();
	//ImGui::RenderPlatformWindowsDefault();
}