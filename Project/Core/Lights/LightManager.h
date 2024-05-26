#pragma once
#include "Light.h"
#include "Mesh/Material.h"

class LightManager
{
public:
    LightManager();
    void Bind(VkCommandBuffer commandBuffer);



private:
    Light m_Light;
    std::shared_ptr<Material> m_ShadowMapPassMaterial;
};
