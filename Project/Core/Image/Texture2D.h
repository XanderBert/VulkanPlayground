#pragma once
#include "Core/ImGuiWrapper.h"
#include "Texture.h"


class Texture2D final : public Texture, public IImGuiRenderable
{
public:
    Texture2D(const std::string &path, ::VulkanContext *vulkanContext);
    Texture2D(Texture2D &&other) noexcept;
    void TransitionAndCopyImageBuffer(VkBuffer srcBuffer) override;
    virtual void Cleanup(VkDevice device) const override;
    virtual void OnImGui() override;
    void InitTexture() override;
private:


    std::unique_ptr<ImGuiTexture> m_ImTexture;
};

