#pragma once
#include "shaders/Logic/Shader.h"

#include "Image/ImageLoader.h"
class DynamicBuffer;


struct GlobalDescriptor
{
	void Init(VulkanContext* vulkanContext);
	void Bind(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout);

	VkDescriptorSetLayout& GetLayout();
	void Cleanup(VkDevice device) const;

private:
	VkDescriptorSetLayout m_GlobalDescriptorSetLayout{};
	VkDescriptorSet m_GlobalDescriptorSet{};
	DynamicBuffer m_GlobalBuffer{};

	uint16_t projectionHandle = 0;
	uint16_t viewProjectionHandle = 0;
	uint16_t cameraHandle = 0;

	//TODO Move out of global descriptor
	Texture* m_Texture;
	VkDescriptorSetLayout m_DescriptorSetLayout{};
	VkDescriptorSet m_DescriptorSet{};
};
