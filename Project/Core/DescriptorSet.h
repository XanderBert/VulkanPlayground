#pragma once
#include <string>
#include <unordered_map>
#include <variant>


#include "Descriptor.h"
#include "DynamicUniformBuffer.h"
#include "Logger.h"
#include "Image/Texture.h"


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

    void AddTexture(int binding, const std::variant<std::filesystem::path,ImageInMemory>& pathOrImage, VulkanContext* pContext, ColorType colorType = ColorType::LINEAR, TextureType textureType = TextureType::TEXTURE_2D)
    {
        // Check if the binding already exists in the uniform buffer map
        if (const auto it = m_UniformBuffers.find(binding); it != m_UniformBuffers.end())
        {
            LogError("Binding at: " + std::to_string(binding) + " is allready used for a UniformBuffer");
            return;
        }

        //Create a new texture
        std::unique_ptr<Texture> texture = std::make_unique<Texture>(pathOrImage, pContext, colorType, textureType);

        //Add a new texture at binding x
        const auto [iterator, isEmplaced] = m_Textures.try_emplace(binding, std::move(texture));
        if (!isEmplaced)
        {
            LogError("Binding at:" + std::to_string(binding) + " is allready used for a Texture");
            return;
        }

        m_DescriptorBuilder.AddBinding(binding, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    }

    void Initialize(VulkanContext* pContext);
    void Bind(VulkanContext *pContext, const VkCommandBuffer& commandBuffer, const VkPipelineLayout & pipelineLayout, int descriptorSetIndex, bool fullRebind = false);

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