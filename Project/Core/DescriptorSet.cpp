#include "DescriptorSet.h"

#include <ranges>

#include "ColorAttachment.h"
#include "DynamicUniformBuffer.h"
#include "GlobalDescriptor.h"
#include "SwapChain.h"
#include "Image/Texture.h"


void DescriptorSet::Initialize(const VulkanContext *pContext)
{
    // Build the descriptor set layout -> Can be done in the ctor?
    m_DescriptorWriter.UpdateSet(pContext->device, m_DescriptorSet);
    m_DescriptorBuilder.Build(pContext->device, m_DescriptorSetLayout);
}

void DescriptorSet::Cleanup(const VulkanContext *pContext)
{
	//Cleanup all the textures and Buffers
    for (DescriptorResource& resource : m_Resources)
    {
        std::visit([pContext](auto& res)
        {
			res.Cleanup(pContext->device);
        }, resource.resource);
    }

    m_Resources.clear();
    vkDestroyDescriptorSetLayout(pContext->device, m_DescriptorSetLayout, nullptr);
}

void DescriptorSet::Bind(VulkanContext *pContext, const VkCommandBuffer &commandBuffer, PipelineType pipelineType)
{
    //Only perform a memcpy for the Uniform Buffers
    //TODO: This memcpy should only be performed once actual data is updated?
	for (auto& resource : m_Resources)
	{
		std::visit([]<typename T0>(T0& res)
		{
			using T = std::decay_t<T0>;
			if constexpr (std::is_same_v<T, DynamicBuffer>)
			{
				res.Bind();
			}
		}, resource.resource);
	}

    vkCmdBindDescriptorSets(commandBuffer, static_cast<VkPipelineBindPoint>(pipelineType), GlobalDescriptor::GetPipelineLayout(), 0, 1, &m_DescriptorSet, 0, nullptr);
}

DescriptorResourceHandle DescriptorSet::AddResource(const DescriptorResource &resource)
{
    //Get the index
    auto newHandle = static_cast<DescriptorResourceHandle>(m_Resources.size());

    //Store the resource
    m_Resources.push_back(resource);

    //Write the resource
    std::visit<DescriptorResource>([&](auto& res)
    {
        res.Write(m_DescriptorWriter, newHandle, resource.type);
    }, resource.resource);


    return newHandle;
}

const DescriptorResource & DescriptorSet::GetResource(int index) const
{
    return m_Resources.at(index);
}

const VkDescriptorSetLayout & DescriptorSet::GetDescriptorSetLayout() const
{
    LogAssert(m_DescriptorSetLayout != VK_NULL_HANDLE, "Make sure to Initialize the DescriptorSet before getting the Layout", true);
    return m_DescriptorSetLayout;
}
