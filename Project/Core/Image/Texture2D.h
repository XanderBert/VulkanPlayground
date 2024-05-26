#pragma once
#include "Core/ImGuiWrapper.h"
#include "Texture.h"


class Texture2D final : public Texture, public IImGuiRenderable
{
public:
    Texture2D(const std::filesystem::path& path, ::VulkanContext *vulkanContext, ColorType colorType);
    Texture2D(const LoadedImage& loadedImage, ::VulkanContext *vulkanContext, ColorType colorType);

    Texture2D(Texture2D &&other) noexcept;
    void TransitionAndCopyImageBuffer(VkBuffer srcBuffer);
    virtual void Cleanup(VkDevice device) const override;
    virtual void OnImGui() override;

    void InitTexture(std::optional<LoadedImage> loadedImage) override;
private:
    std::unique_ptr<ImGuiTexture> m_ImTexture;
};

