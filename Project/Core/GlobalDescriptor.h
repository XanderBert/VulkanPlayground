#pragma once
#include "shaders/Logic/Shader.h"
#include "Image/ImageLoader.h"
#include "Descriptor.h"
#include "DynamicUniformBuffer.h"


struct GlobalDescriptor
{
	static void Init(VulkanContext* vulkanContext);

	
	static void Bind(VulkanContext* vulkanContext, const VkCommandBuffer commandBuffer, const VkPipelineLayout& pipelineLayout);

	static VkDescriptorSetLayout& GetLayout();
	static VkDescriptorSet& GetDescriptorSet();

	static void Cleanup(VkDevice device);

private:
	static inline VkDescriptorSetLayout m_GlobalDescriptorSetLayout{};
	static inline VkDescriptorSet m_GlobalDescriptorSet{};
	static inline DynamicBuffer m_GlobalBuffer{};

	static inline uint16_t projectionHandle = 0;
	static inline uint16_t viewProjectionHandle = 0;
	static inline uint16_t cameraHandle = 0;


	static inline Descriptor::DescriptorWriter m_Writer{};
};