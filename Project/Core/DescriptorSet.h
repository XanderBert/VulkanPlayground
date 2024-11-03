#pragma once
#include <string>
#include <unordered_map>
#include <variant>




#include "ColorAttachment.h"
#include "Descriptor.h"
#include "DynamicUniformBuffer.h"
#include "Logger.h"
#include "Image/Texture.h"




enum class PipelineType;
class DepthResource;
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

	//=========UBO=========
    DynamicBuffer* AddBuffer(int binding, DescriptorType type);
    [[nodiscard]] DynamicBuffer* GetBuffer(int binding);

	//=========Texture=========
    void AddTexture(int binding, const std::variant<std::filesystem::path,ImageInMemory>& pathOrImage, VulkanContext* pContext, ColorType colorType = ColorType::LINEAR, TextureType textureType = TextureType::TEXTURE_2D);
    void AddTexture(int binding, std::shared_ptr<Texture> texture, VulkanContext* pContext);
	void AddGBuffer(int depthBinding, int normalBinding);



	std::shared_ptr<Texture> CreateOutputTexture(int binding,VulkanContext* pContext, const glm::ivec2& extent, ColorType colorType = ColorType::LINEAR, TextureType textureType = TextureType::TEXTURE_2D);
    [[nodiscard]] std::vector<std::shared_ptr<Texture>> GetTextures();

	//TODO, Now a material owns the attachments -> It is a pass that should own the attachments
	//=========Attachments=========
	void AddColorAttachment(ColorAttachment* colorAttachment, int binding);

    void Initialize(const VulkanContext* pContext);
    void Bind(VulkanContext *pContext, const VkCommandBuffer& commandBuffer, const VkPipelineLayout & pipelineLayout, int descriptorSetIndex, PipelineType pipelineType, bool fullRebind = false);

    //Layout to specify in the pipeline layout
    VkDescriptorSetLayout &GetLayout(const VulkanContext* pContext);

    void CleanUp(VkDevice device);

    void OnImGui();
private:

    [[nodiscard]] bool IsBindingUsedForBuffers(int binding) const;
    [[nodiscard]] bool IsBindingUsedForTextures(int binding) const;
    [[nodiscard]] bool IsBindingUsedForDepthTexture(int binding) const;


    std::unordered_map<int, DynamicBuffer> m_Buffers{};
    std::unordered_map<int, std::shared_ptr<Texture>> m_Textures{};
	std::unordered_map<int, ColorAttachment*> m_ColorAttachments{};

	//G Buffer Bindings
	//TODO: clean this up
    int m_DepthTextureBinding{-1};
	int m_NormalTextureBinding{-1};

    VkDescriptorSetLayout m_DescriptorSetLayout{};
    VkDescriptorSet m_DescriptorSet{};

    Descriptor::DescriptorWriter m_DescriptorWriter{};
    Descriptor::DescriptorBuilder m_DescriptorBuilder{};
};