#pragma once
#include <string>
#include <unordered_map>
#include <variant>


#include "Descriptor.h"
#include "DynamicUniformBuffer.h"
#include "Logger.h"
#include "Image/Texture.h"


class VulkanContext;

enum class DescriptorType
{
    UniformBuffer = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
    StorageBuffer = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
};


class DescriptorSet
{
public:
    DescriptorSet() = default;
    ~DescriptorSet() = default;

    DescriptorSet(const DescriptorSet&) = delete;
    DescriptorSet& operator=(const DescriptorSet&) = delete;
    DescriptorSet(DescriptorSet&&) = delete;
    DescriptorSet& operator=(DescriptorSet&&) = delete;

    DynamicBuffer* AddBuffer(int binding, DescriptorType type);
    [[nodiscard]] DynamicBuffer* GetBuffer(int binding);

    void AddTexture(int binding, const std::variant<std::filesystem::path,ImageInMemory>& pathOrImage, VulkanContext* pContext, ColorType colorType = ColorType::LINEAR, TextureType textureType = TextureType::TEXTURE_2D);

    void Initialize(const VulkanContext* pContext);
    void Bind(VulkanContext *pContext, const VkCommandBuffer& commandBuffer, const VkPipelineLayout & pipelineLayout, int descriptorSetIndex, bool fullRebind = false);

    //Layout to specify in the pipeline layout
    VkDescriptorSetLayout &GetLayout(const VulkanContext* pContext);

    void CleanUp(VkDevice device);

    void OnImGui();
private:
    std::unordered_map<int, DynamicBuffer> m_Buffers{};
    std::unordered_map<int, std::unique_ptr<Texture>> m_Textures{};

    VkDescriptorSetLayout m_DescriptorSetLayout{};
    VkDescriptorSet m_DescriptorSet{};

    Descriptor::DescriptorWriter m_DescriptorWriter{};
    Descriptor::DescriptorBuilder m_DescriptorBuilder{};
};