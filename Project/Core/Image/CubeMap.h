#pragma once
#include <ktx.h>

#include "Texture.h"


class CubeMap final : public Texture
{
public:
    CubeMap(const std::string &path, ::VulkanContext *vulkanContext);

    explicit CubeMap(Texture &&other) noexcept : Texture(std::move(other)) {}
    void TransitionAndCopyImageBuffer(VkBuffer srcBuffer) override;
    void InitTexture() override;
private:


    ktxTexture* m_LoadingTexture = nullptr;
};




