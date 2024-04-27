#pragma once
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>
#include <string>
#include <unordered_map>
#include <vector>
#include <vulkan/vulkan.h>

#include "Descriptor.h"

class DynamicBuffer;
class Texture;
class VulkanContext;

//For now we will assume ubo's will only bind to the vertex shader and textures will only bind to the fragment shader
class DescriptorSet
{
public:
    DescriptorSet() = default;
    ~DescriptorSet() = default;

    DescriptorSet(const DescriptorSet&) = delete;
    DescriptorSet& operator=(const DescriptorSet&) = delete;
    DescriptorSet(DescriptorSet&&) = delete;
    DescriptorSet& operator=(DescriptorSet&&) = delete;

    DynamicBuffer* AddUniformBuffer(int binding);
    DynamicBuffer* GetUniformBuffer(int binding);

    void AddTexture(int binding, const std::string& path, VulkanContext* pContext);

    void Initialize(VulkanContext* pContext);
    void Bind(VulkanContext *pContext, const VkCommandBuffer& commandBuffer, const VkPipelineLayout & pipelineLayout, int descriptorSetIndex);

    //Layout to specify in the pipeline layout
    VkDescriptorSetLayout &GetLayout(VulkanContext* pContext);

    void CleanUp(VkDevice device);
private:
    std::unordered_map<int, DynamicBuffer> m_UniformBuffers{};
    std::unordered_map<int, Texture> m_Textures{};

    VkDescriptorSetLayout m_DescriptorSetLayout{};
    VkDescriptorSet m_DescriptorSet{};

    Descriptor::DescriptorWriter m_DescriptorWriter{};
    Descriptor::DescriptorBuilder m_DescriptorBuilder{};
};