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


	static void Bind(VulkanContext* vulkanContext, VkCommandBuffer commandBuffer, const VkPipelineLayout& pipelineLayout, PipelineType pipelineType = PipelineType::Graphics);
	static VkDescriptorSetLayout& GetLayout();
	static void Cleanup(VkDevice device);

    static void OnImGui();

private:
	static inline VkDescriptorSetLayout m_GlobalDescriptorSetLayout{};
	static inline VkDescriptorSet m_GlobalDescriptorSet{};
	static inline DynamicBuffer m_GlobalBuffer{};

	static inline uint16_t inverseProjectionHandle = 0;
	static inline uint16_t viewProjectionHandle = 0;
	static inline uint16_t cameraHandle = 0;
	static inline uint16_t cameraPlaneHandle = 0;

    static inline uint16_t lightPositionHandle = 0;
    static inline uint16_t lightColorHandle = 0;

	static inline Descriptor::DescriptorWriter m_Writer{};
};
