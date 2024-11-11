#pragma once
#include <optional>
#include <vk_mem_alloc.h>



class Texture;
class VulkanContext;
struct Allocator
{
    static void CreateAllocator(const VulkanContext* vulkanContext);
    static void Cleanup(VkDevice device);
    static void GenerateMemoryLayout(VulkanContext* vulkanContext);
    inline static VmaAllocator vmaAllocator;

    inline static std::optional<Texture*> MemoryLayoutTexture = std::nullopt; // This is a placeholder for the texture that will be used to display the memory layout
};
