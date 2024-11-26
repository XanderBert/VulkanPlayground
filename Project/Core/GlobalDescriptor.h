#pragma once
#include "Descriptor.h"
#include "DynamicUniformBuffer.h"
#include "Image/ImageLoader.h"
#include "Lights/Light.h"
#include "Lights/LightManager.h"
#include "shaders/Logic/Shader.h"


struct GlobalDescriptor
{
    GlobalDescriptor() = delete;
    ~GlobalDescriptor() = default;

    GlobalDescriptor(const GlobalDescriptor&) = delete;
    GlobalDescriptor(GlobalDescriptor&&) = delete;
    GlobalDescriptor& operator=(const GlobalDescriptor&) = delete;
    GlobalDescriptor& operator=(GlobalDescriptor&&) = delete;

	static void Init(VulkanContext* vulkanContext);
	static void Bind(VkCommandBuffer commandBuffer);
	static void Cleanup(VkDevice device);
    static void OnImGui();

	[[nodiscard]] static const VkDescriptorSetLayout& GetLayout();
	[[nodiscard]] static VkPipelineLayout& GetPipelineLayout();
	[[nodiscard]] static DescriptorSet& GetDescriptorSet();

private:
	inline static VulkanContext* m_pVulkanContext{};
	inline static DescriptorSet m_DescriptorSet{};
	inline static VkPipelineLayout m_PipelineLayout{};

	static inline uint16_t inverseProjectionHandle = 0;
	static inline uint16_t viewProjectionHandle = 0;
	static inline uint16_t cameraHandle = 0;
	static inline uint16_t cameraPlaneHandle = 0;

    static inline uint16_t lightPositionHandle = 0;
    static inline uint16_t lightColorHandle = 0;
	static inline uint16_t viewMatrixHandle = 0;
};
