#pragma once
#define IMGUI_DEFINE_MATH_OPERATORS

#include "imgui.h"
#include "ImGuizmo.h"
#include <vulkan/vulkan.h>



class ImGuiWrapper final
{
public:

	ImGuiWrapper() = default;
	~ImGuiWrapper() = default;

	ImGuiWrapper(const ImGuiWrapper&) = delete;
	ImGuiWrapper& operator=(const ImGuiWrapper&) = delete;
	ImGuiWrapper(ImGuiWrapper&&) = delete;
	ImGuiWrapper& operator=(ImGuiWrapper&&) = delete;

	static void Initialize(VkQueue graphicsQueue);
	static void Cleanup();
	static void NewFrame();
	static void EndFrame();

    static void SetDarkStyle();
	static void SetDraculaStyle();

private:
	inline static VkDescriptorPool descriptorPool;
};

struct ImGuizmoHandler
{
    inline static ImGuizmo::OPERATION GizmoOperation = ImGuizmo::TRANSLATE;

    static void RotateOperation() { GizmoOperation = ImGuizmo::ROTATE; }
    static void TranslateOperation() { GizmoOperation = ImGuizmo::TRANSLATE; }
    static void ScaleOperation() { GizmoOperation = ImGuizmo::SCALE; }
};

class IImGuiRenderable {
public:
    virtual ~IImGuiRenderable() = default;
    virtual void OnImGui() = 0;
};

class ImGuiTexture final
{
public:
    ImGuiTexture(VkSampler sampler, VkImageView imageView, ImVec2 size);
    ImGuiTexture(const ImGuiTexture&) = delete;
    ImGuiTexture& operator=(const ImGuiTexture&) = delete;
    ImGuiTexture(ImGuiTexture&&) = delete;
    ImGuiTexture& operator=(ImGuiTexture&& other) noexcept
    {
        if(&other != this)
        {
            ImGuiDescriptorSet = other.ImGuiDescriptorSet;
        }

        return *this;
    }

    void Cleanup();
    void SetNewSize(ImVec2 newSize);
    void OnImGui();
private:
    VkDescriptorSet ImGuiDescriptorSet;
    ImVec2 m_Size;

};







