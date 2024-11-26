#include "DescriptorSet.h"

#include <algorithm>
#include <ranges>

#include "ColorAttachment.h"
#include "DynamicUniformBuffer.h"
#include "GlobalDescriptor.h"
#include "SwapChain.h"
#include "Image/Texture.h"


void DescriptorSet::Initialize(const VulkanContext *pContext) {
    // Iterate over all resources and initialize those that need to be initialized
    for (auto& resource : m_Resources)
    {
        std::visit<DescriptorResource>([&]<typename T0>(T0& res)
        {
            using T = std::decay_t<T0>;

            //Initialize the DynamicBuffers
            if constexpr (std::is_same_v<T, DynamicBuffer>)
            {
                res.Init();
            }
        }, resource);


        // Build the descriptor set layout
        m_DescriptorBuilder.Build(pContext->device, m_DescriptorSetLayout);
    }
}

void DescriptorSet::Cleanup(const VulkanContext *pContext) {
    //Cleanup all the textures and Buffers
    for (auto& resource : m_Resources)
    {
        std::visit<DescriptorResource>([&]<typename T0>(T0& res)
        {
            using T = std::decay_t<T0>;
            if constexpr (std::is_same_v<T, DynamicBuffer>)
            {
                res.Cleanup(pContext->device);
            }
            else if constexpr (std::is_same_v<T, std::shared_ptr<Texture>>)
            {

                if(!res->IsPendingKill())
                {
                    res->Cleanup(pContext->device);
                }
            }
        }, resource);
    }

    m_Resources.clear();
    vkDestroyDescriptorSetLayout(pContext->device, m_DescriptorSetLayout, nullptr);
}

void DescriptorSet::Bind(VulkanContext *pContext, const VkCommandBuffer &commandBuffer, PipelineType pipelineType) {
    for (auto& resource : m_Resources)
    {
        std::visit<DescriptorResource>([&]<typename T0>(T0& res)
        {
            using T = std::decay_t<T0>;
            if constexpr (std::is_same_v<T, DynamicBuffer> || std::is_same_v<T, std::shared_ptr<Texture>>)
            {
                res.Bind(m_DescriptorWriter, resource.type);
            }
            else if constexpr (std::is_same_v<T, ColorAttachment*>)
            {
                res.Bind(m_DescriptorWriter);
            }
        }, resource);
    }


    //TODO: the sets don't need to be updated each frame
    m_DescriptorWriter.UpdateSet(pContext->device, m_DescriptorSet);
    vkCmdBindDescriptorSets(commandBuffer, static_cast<VkPipelineBindPoint>(pipelineType), GlobalDescriptor::GetPipelineLayout(), 0, 1, &m_DescriptorSet, 0, nullptr);
}

int DescriptorSet::AddResource(const DescriptorResource &resource) {
    m_Resources.push_back(resource);
    return static_cast<int>(m_Resources.size() - 1);
}

const DescriptorResource & DescriptorSet::GetResource(int index) const {
    return m_Resources.at(index);
}

const VkDescriptorSetLayout & DescriptorSet::GetDescriptorSetLayout() const {

    LogAssert(m_DescriptorSetLayout != VK_NULL_HANDLE, "Make sure to Initialize the DescriptorSet before getting the Layout", true);

    return m_DescriptorSetLayout;
}
