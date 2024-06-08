#include "DescriptorSet.h"

#include <algorithm>
#include <ranges>

#include "DynamicUniformBuffer.h"
#include "Image/ImageLoader.h"

DynamicBuffer *DescriptorSet::AddUniformBuffer(int binding) {
    // check if the binding already exists in the texture map
    if (const auto it = m_Textures.find(binding); it != m_Textures.end()) {
        LogError("Texture already exists at binding " + std::to_string(binding));
        return nullptr;
    }

    // Add a new uniform buffer at binding x
    const auto [iterator, isEmplaced] = m_UniformBuffers.try_emplace(binding, std::move(DynamicBuffer()));

    if (!isEmplaced)
    {
        LogError("Uniform buffer already exists at binding " + std::to_string(binding));
        return nullptr;
    }

    m_DescriptorBuilder.AddBinding(binding, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);


    return &iterator->second;
}
DynamicBuffer *DescriptorSet::GetUniformBuffer(int binding)
{
    //find the uniform buffer at binding x
    //  return nullptr if it does not exist
    const auto it = m_UniformBuffers.find(binding);

    if(it == m_UniformBuffers.end())
    {
        LogWarning("Tried To Acces Uniform buffer at binding: " + std::to_string(binding) + " but it does not exist");
        return nullptr;
    }

    return &it->second;
}



void DescriptorSet::Initialize(VulkanContext *pContext)
{
    for (auto &[binding, ubo]: m_UniformBuffers)
    {
        ubo.Init(pContext);
    }

    m_DescriptorSetLayout = m_DescriptorBuilder.Build(pContext->device, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
}

void DescriptorSet::Bind(VulkanContext *pContext, const VkCommandBuffer &commandBuffer, const VkPipelineLayout &pipelineLayout, int descriptorSetIndex, bool fullRebind )
{

    m_DescriptorSet = Descriptor::DescriptorManager::Allocate(pContext->device, m_DescriptorSetLayout, 0);
    m_DescriptorWriter.Cleanup();

    // Update the data of all the ubo's
    for (auto &[binding, ubo]: m_UniformBuffers)
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

    for (auto &[binding, texture]: m_Textures) {
        texture->ProperBind(binding, m_DescriptorWriter);
    }

    m_DescriptorWriter.UpdateSet(pContext->device, m_DescriptorSet);

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, descriptorSetIndex, 1,
                            &m_DescriptorSet, 0, nullptr);
}

VkDescriptorSetLayout &DescriptorSet::GetLayout(VulkanContext* pContext)
{
    if(m_DescriptorSetLayout == VK_NULL_HANDLE)
    {
        LogWarning("The Descriptor Set Layout is VK_NULL_HANDLE! Did you call DescriptorSet::Initialize(VulkanContext *pContext)?");
        LogInfo("Initializing Descriptor Set Layout...");
        Initialize(pContext);
    }

    LogAssert(m_DescriptorSetLayout != VK_NULL_HANDLE, "The Descriptor Set Layout is VK_NULL_HANDLE! Did you call DescriptorSet::Initialize(VulkanContext *pContext)?", true)
    return m_DescriptorSetLayout;
}

void DescriptorSet::CleanUp(VkDevice device) {
    // Cleanup the uniform buffers
    for (auto &[binding, ubo]: m_UniformBuffers) {
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

    if(!m_UniformBuffers.empty())
    {
        ImGui::Separator();
        ImGui::Text("Uniform Buffers:");
        for(auto& [binding, ubo] : m_UniformBuffers)
        {
            ubo.OnImGui();
        }
    }
}
