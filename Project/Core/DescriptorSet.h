#pragma once
#include <string>
#include <unordered_map>
#include <vulkan/vulkan.h>
#include "Descriptor.h"
#include "DynamicUniformBuffer.h"
#include "Image/CubeMap.h"
#include "Image/ImageLoader.h"
#include "Image/Texture.h"
#include "Image/Texture2D.h"


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

    template<typename TextureType>
    void AddTexture(int binding, const std::string& path, VulkanContext* pContext)
    requires TextureConcept<TextureType>
    {
        // Check if the binding already exists in the uniform buffer map
        if (const auto it = m_UniformBuffers.find(binding); it != m_UniformBuffers.end())
        {
            LogError("Binding at: " + std::to_string(binding) + " is allready used for a UniformBuffer");
            return;
        }

        //Create and add a new texture at binding x
        const auto [iterator, isEmplaced] = m_Textures.try_emplace(binding, std::make_unique<TextureType>(path, pContext));
        if (!isEmplaced)
        {
            LogError("Binding at:" + std::to_string(binding) + " is allready used for a Texture");
            return;
        }

        m_DescriptorBuilder.AddBinding(binding, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    }

    void Initialize(VulkanContext* pContext);
    void Bind(VulkanContext *pContext, const VkCommandBuffer& commandBuffer, const VkPipelineLayout & pipelineLayout, int descriptorSetIndex);

    //Layout to specify in the pipeline layout
    VkDescriptorSetLayout &GetLayout(VulkanContext* pContext);

    void CleanUp(VkDevice device);

    void OnImGui();
private:
    std::unordered_map<int, DynamicBuffer> m_UniformBuffers{};
    std::unordered_map<int, std::unique_ptr<Texture>> m_Textures{};

    VkDescriptorSetLayout m_DescriptorSetLayout{};
    VkDescriptorSet m_DescriptorSet{};

    Descriptor::DescriptorWriter m_DescriptorWriter{};
    Descriptor::DescriptorBuilder m_DescriptorBuilder{};
};