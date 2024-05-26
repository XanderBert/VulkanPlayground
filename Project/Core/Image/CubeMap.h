#pragma once
#include <ktx.h>

#include "Texture.h"


class CubeMap final : public Texture
{
public:
    CubeMap(const std::filesystem::path &path, ::VulkanContext *vulkanContext, ColorType colorType);
    CubeMap(const LoadedImage &loadedImage, ::VulkanContext *vulkanContext, ColorType colorType);

    explicit CubeMap(Texture &&other) noexcept : Texture(std::move(other)) {}
    void TransitionAndCopyImageBuffer(VkBuffer srcBuffer, ktxTexture* texture);
    void InitTexture(std::optional<LoadedImage> loadedImage) override;

};




