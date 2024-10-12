#include "DescriptorSet.h"

#include <algorithm>
#include <ranges>

#include "DepthResource.h"
#include "DynamicUniformBuffer.h"
#include "Image/ImageLoader.h"

DynamicBuffer *DescriptorSet::AddBuffer(int binding, DescriptorType type)
{
    if (IsBindingUsedForTextures(binding) || IsBindingUsedForDepthTexture(binding))
    {
        LogError("(Depth)Texture already exists at binding " + std::to_string(binding));
        return nullptr;
    }


    // Add a new Buffer at binding x
    const auto [iterator, isEmplaced] = m_Buffers.try_emplace(binding, std::move(DynamicBuffer()));

    if (!isEmplaced)
    {
        LogError("Buffer already exists at binding " + std::to_string(binding));
        return nullptr;
    }

    m_DescriptorBuilder.AddBinding(binding, static_cast<VkDescriptorType>(type));

    iterator->second.SetDescriptorType(type);

    return &iterator->second;
}
DynamicBuffer *DescriptorSet::GetBuffer(int binding)
{
    //find the uniform buffer at binding x
    //  return nullptr if it does not exist
    const auto it = m_Buffers.find(binding);

    if(it == m_Buffers.end())
    {
        LogWarning("Tried To Access Uniform buffer at binding: " + std::to_string(binding) + " but it does not exist");
        return nullptr;
    }

    return &it->second;
}

void DescriptorSet::AddTexture(int binding, const std::variant<std::filesystem::path, ImageInMemory> &pathOrImage, VulkanContext *pContext, ColorType colorType, TextureType textureType) {

    if(IsBindingUsedForBuffers(binding) || IsBindingUsedForDepthTexture(binding))
    {
        LogError("Binding at:" + std::to_string(binding) + " is already used for a Buffer or DepthTexture");
        return;
    }

    //Create a new texture
    std::unique_ptr<Texture> texture = std::make_unique<Texture>(pathOrImage, pContext, colorType, textureType);

    //Add a new texture at binding x
    const auto [iterator, isEmplaced] = m_Textures.try_emplace(binding, std::move(texture));
    if (!isEmplaced)
    {
        LogError("Binding at:" + std::to_string(binding) + " is already used for a Texture");
        return;
    }

    m_DescriptorBuilder.AddBinding(binding, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
}

void DescriptorSet::AddDepthTexture(int binding)
{
    if(IsBindingUsedForBuffers(binding) || IsBindingUsedForTextures(binding))
    {
        LogError("Binding at:" + std::to_string(binding) + " is already used for a Buffer or Texture");
        return;
    }

    //Add a new depth texture at binding x
    m_DepthTextureBinding = binding;

    m_DescriptorBuilder.AddBinding(binding, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
}

Texture* DescriptorSet::CreateOutputTexture(int binding, VulkanContext *pContext, const glm::ivec2 &extent,
        ColorType colorType, TextureType textureType)
{
    if(IsBindingUsedForBuffers(binding) || IsBindingUsedForDepthTexture(binding))
    {
        LogError("Binding at:" + std::to_string(binding) + " is already used for a Buffer, Texture or DepthTexture");
        return nullptr;
    }

    //Create a new texture
    std::unique_ptr<Texture> texture = std::make_unique<Texture>(pContext, extent, colorType, textureType);
    texture->SetDescriptorImageType(DescriptorImageType::STORAGE_IMAGE);

    //Add a new texture at binding x
    const auto [iterator, isEmplaced] = m_Textures.try_emplace(binding, std::move(texture));
    if (!isEmplaced)
    {
        LogError("Binding at:" + std::to_string(binding) + " is already used for a Texture");
        return nullptr;
    }

    m_DescriptorBuilder.AddBinding(binding, static_cast<VkDescriptorType>(DescriptorImageType::STORAGE_IMAGE));

    return m_Textures[binding].get();
}



void DescriptorSet::Initialize(const VulkanContext *pContext)
{
    for (auto &[binding, ubo]: m_Buffers)
    {
        ubo.Init();
    }

    m_DescriptorSetLayout = m_DescriptorBuilder.Build(pContext->device, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_COMPUTE_BIT);
}

void DescriptorSet::Bind(VulkanContext *pContext, const VkCommandBuffer& commandBuffer, const VkPipelineLayout & pipelineLayout, int descriptorSetIndex, PipelineType pipelineType, bool fullRebind)
{

    m_DescriptorSet = Descriptor::DescriptorManager::Allocate(pContext->device, m_DescriptorSetLayout, 0);
    m_DescriptorWriter.Cleanup();

    // Update the data of all the ubo's
    //Then bind them
    for (auto &[binding, ubo]: m_Buffers)
    {
        if(fullRebind)
        {
            ubo.FullRebind(binding, m_DescriptorSet, m_DescriptorWriter, pContext);
        }
        else
        {
            ubo.ProperBind(binding, m_DescriptorWriter);
        }
    }

    //Bind all textures
    for (auto &[binding, texture]: m_Textures) {
        texture->ProperBind(binding, m_DescriptorWriter);
    }

    //Bind DepthResource if needed
    if(m_DepthTextureBinding != -1)
    {
        m_DescriptorWriter.WriteImage(m_DepthTextureBinding, DepthResource::GetImageView(), DepthResource::GetSampler(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    }

    m_DescriptorWriter.UpdateSet(pContext->device, m_DescriptorSet);

    vkCmdBindDescriptorSets(commandBuffer, static_cast<VkPipelineBindPoint>(pipelineType), pipelineLayout, descriptorSetIndex, 1,
                            &m_DescriptorSet, 0, nullptr);
}

VkDescriptorSetLayout &DescriptorSet::GetLayout(const VulkanContext* pContext)
{
    if(m_DescriptorSetLayout == VK_NULL_HANDLE)
    {
        //LogWarning("The Descriptor Set Layout is VK_NULL_HANDLE! Did you call DescriptorSet::Initialize(VulkanContext *pContext)?");
        LogInfo("Initializing Descriptor Set Layout...");
        Initialize(pContext);
    }

    LogAssert(m_DescriptorSetLayout != VK_NULL_HANDLE, "The Descriptor Set Layout is VK_NULL_HANDLE! Did you call DescriptorSet::Initialize(VulkanContext *pContext)?", true)
    return m_DescriptorSetLayout;
}

void DescriptorSet::CleanUp(VkDevice device) {
    // Cleanup the uniform buffers
    for (auto &[binding, ubo]: m_Buffers) {
        ubo.Cleanup(device);
    }

    // Cleanup the textures
    for (auto &[binding, texture]: m_Textures) {
        texture->Cleanup(device);
    }



    // Cleanup the layout
    vkDestroyDescriptorSetLayout(device, m_DescriptorSetLayout, nullptr);
}
void DescriptorSet::OnImGui()
{
    if(!m_Textures.empty())
    {
        ImGui::Separator();
        ImGui::Text("Textures:");
        for(const auto &texture: m_Textures | std::views::values)
        {
             texture->OnImGui();
        }
    }

    if(!m_Buffers.empty())
    {
        ImGui::Separator();
        ImGui::Text("Uniform Buffers:");
        for(auto& [binding, ubo] : m_Buffers)
        {
            ubo.OnImGui();
        }
    }
}

bool DescriptorSet::IsBindingUsedForBuffers(int binding) const
{
    const auto it = m_Buffers.find(binding);
    return it != m_Buffers.end();
}

bool DescriptorSet::IsBindingUsedForTextures(int binding) const
{
    const auto it = m_Textures.find(binding);
    return it != m_Textures.end();
}

bool DescriptorSet::IsBindingUsedForDepthTexture(int binding) const
{
    return m_DepthTextureBinding == binding;
}
