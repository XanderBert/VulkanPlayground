#pragma once
#include <string>
#include <vector>
#include <vulkan/vulkan.h>

#include "Core/Logger.h"


#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif


class VulkanContext;
namespace tools {
    VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);

    void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);

    std::vector<char> readFile(const std::string& filename);

    std::string readFileStr(const std::string& filename);

    void writeFileStr(const std::string& filename, const std::string& data);

    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties, VkPhysicalDevice physicalDevice);

    VkAccessFlags GetAccessFlags(VkImageLayout layout);

    VkPipelineStageFlags GetPipelineStageFlags(VkImageLayout layout);

    void InsertImageMemoryBarrier(const VkCommandBuffer commandBuffer, VkImage image, VkAccessFlags srcAccessMask,VkAccessFlags dstAccessMask, VkImageLayout oldImageLayout, VkImageLayout newImageLayout, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkImageSubresourceRange subresourceRange);

    void InsertImageMemoryBarrier(VkCommandBuffer commandBuffer,
        VkImage                        image,
        VkImageLayout                  oldLayout,
        VkImageLayout                  newLayout,
        VkImageSubresourceRange const& subresourceRange);

    void OpenFile(const std::string& path);


    template<typename Interface,typename Class>
    struct ImplementsInterface
    {
        static const bool value = std::is_base_of_v<Interface, Class>;
    };
#define CheckImplementsInterface(Interface, Class) static_assert(tools::ImplementsInterface<Interface, std::remove_pointer_t<decltype(Class)>>::value, "Class does not implement the interface");


}