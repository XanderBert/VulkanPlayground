#include "VulkanBase.h"

#include "Core/CommandPool.h"
#include "Core/ImGuiWrapper.h"
#include "Core/SwapChain.h"
#include "Core/VmaUsage.h"
#include "Input/Input.h"
#include "Mesh/MaterialManager.h"
#include "Patterns/ServiceLocator.h"
#include "Scene/SceneManager.h"
#include "shaders/Logic/ShaderEditor.h"
#include "VulkanTypes.h"
#include "Core/DepthResource.h"


void VulkanBase::run()
{
    ServiceConfigurator::Configure();
    initVulkan();
    ImGuiWrapper::Initialize(m_pContext->graphicsQueue);
    ShaderEditor::Init();
    SceneManager::AddScene(std::make_unique<Scene>(m_pContext));
    Input::SetupInput(m_pContext->window.Get());
    MaterialManager::CreatePipelines();
    mainLoop();
    cleanup();
}


void VulkanBase::initVulkan()
{
    m_pContext = ServiceLocator::GetService<VulkanContext>();
    createInstance();
    setupDebugMessenger();

    SwapChain::CreateSurface(m_pContext);
    pickPhysicalDevice();
    createLogicalDevice();

    Allocator::CreateAllocator(m_pContext);

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

void VulkanBase::mainLoop()
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

void VulkanBase::cleanup() const
{
    VkDevice device = m_pContext->device;

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
    Allocator::Cleanup(m_pContext->device);
    ImGuiWrapper::Cleanup();
    SwapChain::Cleanup(m_pContext);
    m_pContext->CleanUp();
}


