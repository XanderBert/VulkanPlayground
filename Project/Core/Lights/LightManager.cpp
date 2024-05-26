#include "LightManager.h"

#include "Mesh/MaterialManager.h"
#include "Patterns/ServiceLocator.h"

LightManager::LightManager()
{
    m_ShadowMapPassMaterial =
            MaterialManager::CreateMaterial(ServiceLocator::GetService<VulkanContext>(), "ShadowMapPass.vert",
                                            "ShadowMapPass.frag", "ShadowMapPassMaterial");

    auto descriptor = m_ShadowMapPassMaterial->GetDescriptorSet();

}
void LightManager::Bind(VkCommandBuffer commandBuffer)
{
    m_ShadowMapPassMaterial->Bind(commandBuffer, m_Light.GetLightSpace());
}
