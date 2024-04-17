#pragma once

#include <vulkan/vulkan_core.h>


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








