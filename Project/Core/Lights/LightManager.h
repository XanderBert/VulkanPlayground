#pragma once
#include "Light.h"
#include "Mesh/Material.h"

class LightManager final
{
public:
    LightManager() = default;
    ~LightManager() = default;

    static std::vector<Light*> GetLights();

    static void OnImGui();
    //static void Bind(VkCommandBuffer commandBuffer);



private:
    inline static Light m_Light{};
    //std::shared_ptr<Material> m_ShadowMapPassMaterial;
};
