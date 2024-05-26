#pragma once
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <vulkan/vulkan_core.h>

#include "Core/ImGuiWrapper.h"


class VulkanContext;
class Light final : public IImGuiRenderable {
public:
    Light() = default;
    explicit Light(const VulkanContext *vulkanContext);
    ~Light() = default;

    // Light(const Light&) = delete;
    // Light(Light&&) = delete;
    // Light& operator=(const Light&) = delete;
    // Light& operator=(Light&&) = delete;

    virtual void OnImGui() override;

    VkImageView GetShadowMapView() const;
    VkSampler GetShadowMapSampler() const;

    void TransitionShadowMapToRead(VkCommandBuffer commandBuffer);
    void TransitionShadowMapToWrite(VkCommandBuffer commandBuffer);

    float *GetPosition();
    float *GetDirection();
    glm::mat4 GetLightSpace();
    float *GetColor();
    float &GetIntensity();

    VkRenderingAttachmentInfoKHR GetAttachmentInfo() const;
private:
    glm::vec3 m_Position{};
    glm::vec3 m_Color{};
    glm::vec3 m_Direction{};

    float m_Intensity{};

    glm::mat4 m_LightSpaceMatrix{};

    VkSampler m_ShadowMapSampler{};
    VkImageView m_ShadowMapView{};
    VkImage m_ShadowMapImage{};
    VkDeviceMemory m_ShadowMapMemory{};
    VkFormat m_Format;

    bool isSetup = false;
};




