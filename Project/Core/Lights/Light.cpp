#include "Light.h"
#include <glm/gtc/type_ptr.hpp>

#include "Core/DepthResource.h"
#include "Core/Image/ImageLoader.h"


Light::Light(const VulkanContext *vulkanContext) :
    m_Position(), m_Color({1, 1, 1}), m_Direction({0, 1, 0}), m_Intensity(1), m_LightSpaceMatrix(glm::mat4(1))

{
    DepthResourceBuilder::Build(vulkanContext, m_ShadowMapImage, m_ShadowMapView, m_ShadowMapMemory, m_Format);
    Image::CreateSampler(vulkanContext, m_ShadowMapSampler, 1);
}

void Light::OnImGui()
{
    ImGui::Text("Light");


    ImGui::Text("Position: %f %f %f", m_Position.x, m_Position.y, m_Position.z);
    ImGui::Text("Color: %f %f %f", m_Color.x, m_Color.y, m_Color.z);
    ImGui::Text("Direction: %f %f %f", m_Direction.x, m_Direction.y, m_Direction.z);
    ImGui::Text("Intensity: %f", m_Intensity);
    ImGui::Text("LightSpaceMatrix: ");
    for (int i = 0; i < 4; i++)
    {
        ImGui::Text("%f %f %f %f", m_LightSpaceMatrix[i][0], m_LightSpaceMatrix[i][1], m_LightSpaceMatrix[i][2], m_LightSpaceMatrix[i][3]);

    }
    ImGui::Text("ShadowMapSampler: %d", m_ShadowMapSampler);
    ImGui::Text("ShadowMapView: %d", m_ShadowMapView);
    ImGui::Text("ShadowMapImage: %d", m_ShadowMapImage);
    ImGui::Text("ShadowMapMemory: %d", m_ShadowMapMemory);


    static ImGuiTexture m_ImGuiTexture = ImGuiTexture(m_ShadowMapSampler, m_ShadowMapView, {100,100});;

    if(isSetup)
    {
        m_ImGuiTexture.Cleanup();
    }

    isSetup = true;
    m_ImGuiTexture = ImGuiTexture(m_ShadowMapSampler, m_ShadowMapView, {100,100});
    m_ImGuiTexture.OnImGui();
}

VkImageView Light::GetShadowMapView() const
{
    return m_ShadowMapView;
}

VkSampler Light::GetShadowMapSampler() const
{
    return m_ShadowMapSampler;
}
void Light::TransitionShadowMapToRead(VkCommandBuffer commandBuffer)
{
    tools::InsertImageMemoryBarrier(
            commandBuffer,
            m_ShadowMapImage,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VkImageSubresourceRange{ VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1 });
}

void Light::TransitionShadowMapToWrite(VkCommandBuffer commandBuffer)
{
    tools::InsertImageMemoryBarrier(
            commandBuffer,
            m_ShadowMapImage,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            VkImageSubresourceRange{ VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1 });
}

float *Light::GetPosition() { return value_ptr(m_Position); }
float *Light::GetDirection()
{
    return value_ptr(m_Direction);
}

 glm::mat4 Light::GetLightSpace()
{
    return m_LightSpaceMatrix;
}

float *Light::GetColor()
{
    return value_ptr(m_Color);
}

float& Light::GetIntensity()
{
    return m_Intensity;
}

VkRenderingAttachmentInfoKHR Light::GetAttachmentInfo() const
{
    VkRenderingAttachmentInfoKHR depthAttachmentInfo{};
    depthAttachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
    depthAttachmentInfo.pNext = VK_NULL_HANDLE;
    depthAttachmentInfo.imageView = m_ShadowMapView;
    depthAttachmentInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depthAttachmentInfo.resolveMode = VK_RESOLVE_MODE_NONE;
    depthAttachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depthAttachmentInfo.clearValue = {{1.0f, 0.0f}};

    return depthAttachmentInfo;
}
