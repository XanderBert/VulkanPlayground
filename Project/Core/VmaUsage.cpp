#define WIN32_LEAN_AND_MEAN
#define VK_USE_PLATFORM_WIN32_KHR
#define VMA_VULKAN_VERSION 1003000 // Vulkan 1.3
#define VMA_IMPLEMENTATION

#include "VmaUsage.h"

#include <vk_mem_alloc.h>

#include "Image/Texture.h"
#include "vulkanbase/VulkanTypes.h"
#include "vulkanbase/VulkanUtil.h"

void Allocator::CreateAllocator(const VulkanContext *vulkanContext)
{
    VmaVulkanFunctions vulkanFunctions = {};
    vulkanFunctions.vkGetInstanceProcAddr = &vkGetInstanceProcAddr;
    vulkanFunctions.vkGetDeviceProcAddr = &vkGetDeviceProcAddr;

    VmaAllocatorCreateInfo allocatorCreateInfo{};
    allocatorCreateInfo.vulkanApiVersion = VK_API_VERSION_1_3;
    allocatorCreateInfo.physicalDevice = vulkanContext->physicalDevice;
    allocatorCreateInfo.device = vulkanContext->device;
    allocatorCreateInfo.instance = vulkanContext->instance;
    allocatorCreateInfo.pVulkanFunctions = &vulkanFunctions;

    vmaCreateAllocator(&allocatorCreateInfo, &VmaAllocator);
}

void Allocator::Cleanup(VkDevice device)
{
    if(MemoryLayoutTexture.has_value())
    {
        MemoryLayoutTexture.value()->Cleanup(device);
        delete MemoryLayoutTexture.value();
    }
    vmaDestroyAllocator(VmaAllocator);


}

void Allocator::GenerateMemoryLayout(VulkanContext*  vulkanContext)
{
    char *statsString = nullptr;
    vmaBuildStatsString(VmaAllocator, &statsString, true);
    tools::writeFileStr("MemoryLayout.json", statsString);
    vmaFreeStatsString(VmaAllocator, statsString);

    //TODO: Instead of using the python script to generate the image we should use ImGui to render the memory layout
    //python GpuMemDumpVis.py -o OUTPUT_FILE INPUT_FILE
    //std::filesystem::path currentPath = std::filesystem::current_path();
    //std::filesystem::path pythonScriptPath = currentPath.parent_path().parent_path().generic_string() + "/includes/VulkanMemoryAllocator/tools/GpuMemDumpVis";
    //std::string command = "python " + pythonScriptPath.generic_string() + "\\GpuMemDumpVis.py -o MemoryLayout.png " + currentPath.generic_string() + "\\MemoryLayout.json";
    //system(command.c_str());

    //MemoryLayoutTexture = new Texture(currentPath.generic_string() + "/MemoryLayout.png", vulkanContext , ColorType::SRGB, TextureType::TEXTURE_2D);
    //MemoryLayoutTexture.value()->SetIsChangeable(false);
}
