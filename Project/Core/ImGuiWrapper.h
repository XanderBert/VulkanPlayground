#pragma once

#include <vulkan/vulkan_core.h>

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "ImGuizmo.h"


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

private:
	inline static VkDescriptorPool descriptorPool;
};

struct ImGuizmoHandler
{
    inline static ImGuizmo::OPERATION GizmoOperation = ImGuizmo::TRANSLATE;

    inline static void RotateOperation() { GizmoOperation = ImGuizmo::ROTATE; }
    inline static void TranslateOperation() { GizmoOperation = ImGuizmo::TRANSLATE; }
    inline static void ScaleOperation() { GizmoOperation = ImGuizmo::SCALE; }
};








