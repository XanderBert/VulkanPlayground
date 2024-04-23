#include "Descriptor.h"
#include <algorithm>
#include "GlobalDescriptor.h"
#include "SwapChain.h"

namespace Descriptor
{

	// _____                     _       _                        _ _                 _             
	//|  __ \                   (_)     | |                 /\   | | |               | |            
	//| |  | | ___ ___  ___ _ __ _ _ __ | |_ ___  _ __     /  \  | | | ___   ___ __ _| |_ ___  _ __ 
	//| |  | |/ _ / __|/ __| '__| | '_ \| __/ _ \| '__|   / /\ \ | | |/ _ \ / __/ _` | __/ _ \| '__|
	//| |__| |  __\__ | (__| |  | | |_) | || (_) | |     / ____ \| | | (_) | (_| (_| | || (_) | |   
	//|_____/ \___|___/\___|_|  |_| .__/ \__\___/|_|    /_/    \_|_|_|\___/ \___\__,_|\__\___/|_|   
    //                          | |                                                               
    //                          |_|                                                               
	//


	void DescriptorAllocator::Init(VkDevice device, uint32_t maxSets, const std::vector<PoolSizeRatio>& poolRatios)
	{
		m_PoolSizeRatios.clear();

		if(poolRatios.empty())
		{
			//TODO: Add more types
			m_PoolSizeRatios.push_back({ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1.0f });
			m_PoolSizeRatios.push_back({ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1.0f });
		}
		else
		{
			m_PoolSizeRatios = poolRatios;
		}


		const VkDescriptorPool newPool = CreatePool(device, maxSets, poolRatios);

		setsPerPool = maxSets * 1.5; //grow it next allocation

		m_ReadyPools.push_back(newPool);
	}

	void DescriptorAllocator::ClearPools(VkDevice device)
	{
		for (const auto pool : m_ReadyPools) 
		{
			vkResetDescriptorPool(device, pool, 0);
		}


		for (auto pool : m_FullPools) 
		{
			vkResetDescriptorPool(device, pool, 0);
			m_ReadyPools.push_back(pool);
		}

		m_FullPools.clear();
	}

	VkDescriptorSet DescriptorAllocator::Allocate(VkDevice device, VkDescriptorSetLayout layout)
	{
		//get or create a pool to allocate from
		VkDescriptorPool poolToUse = GrabPool(device);

		VkDescriptorSetAllocateInfo allocInfo = {};
		allocInfo.pNext = nullptr;
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = poolToUse;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &layout;

		VkDescriptorSet descriptorSet;
		const VkResult result = vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet);

		//allocation failed. Try again
		if (result == VK_ERROR_OUT_OF_POOL_MEMORY || result == VK_ERROR_FRAGMENTED_POOL) {

			m_FullPools.push_back(poolToUse);

			poolToUse = GrabPool(device);
			allocInfo.descriptorPool = poolToUse;

			VulkanCheck(vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet), "Failed To Allocate Descriptor Set!")
		}

		m_ReadyPools.push_back(poolToUse);
		return descriptorSet;
	}

	void DescriptorAllocator::Cleanup(VkDevice device)
	{
		for (const auto pool : m_ReadyPools) 
		{
			vkDestroyDescriptorPool(device, pool, nullptr);
		}

		m_ReadyPools.clear();

		for (const auto pool : m_FullPools) 
		{
			vkDestroyDescriptorPool(device, pool, nullptr);
		}

		m_FullPools.clear();
	}

	VkDescriptorPool DescriptorAllocator::GrabPool(VkDevice device)
	{
		constexpr int maxSets = 4096;
		constexpr float setMultiplier = 1.5f;

		VkDescriptorPool newPool;

		if (!m_ReadyPools.empty())
		{
			newPool = m_ReadyPools.back();
			m_ReadyPools.pop_back();
		}
		else 
		{
			//Create a new pool
			newPool = CreatePool(device, setsPerPool, m_PoolSizeRatios);

			setsPerPool = setsPerPool * setMultiplier;
			if (setsPerPool > maxSets)
			{
				setsPerPool = maxSets;
			}
		}

		return newPool;
	}

	VkDescriptorPool DescriptorAllocator::CreatePool(VkDevice device, uint32_t setCount, const std::vector<PoolSizeRatio>& poolRatios)
	{
		std::vector<VkDescriptorPoolSize> poolSizes;

		for (const PoolSizeRatio ratio : poolRatios) 
		{
			VkDescriptorPoolSize poolSize;
			poolSize.type = ratio.type;
			poolSize.descriptorCount = ratio.ratio * setCount;

			poolSizes.emplace_back(poolSize);
		}

		VkDescriptorPoolCreateInfo pool_info = {};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.flags = 0;
		pool_info.maxSets = setCount;
		pool_info.poolSizeCount = (uint32_t)poolSizes.size();
		pool_info.pPoolSizes = poolSizes.data();

		VkDescriptorPool newPool;
		vkCreateDescriptorPool(device, &pool_info, nullptr, &newPool);
		return newPool;
	}



	//_____                     _       _              __          __   _ _            
	//|  __ \                   (_)     | |             \ \        / /  (_| |           
	//| |  | | ___ ___  ___ _ __ _ _ __ | |_ ___  _ __   \ \  /\  / _ __ _| |_ ___ _ __ 
	//| |  | |/ _ / __|/ __| '__| | '_ \| __/ _ \| '__|   \ \/  \/ | '__| | __/ _ | '__|
	//| |__| |  __\__ | (__| |  | | |_) | || (_) | |       \  /\  /| |  | | ||  __| |   
	//|_____/ \___|___/\___|_|  |_| .__/ \__\___/|_|        \/  \/ |_|  |_|\__\___|_|   
	//		                      | |                                                   
    //	                          |_|                                                   

    void DescriptorWriter::WriteImage(int binding, VkImageView image, VkSampler sampler, VkImageLayout layout,
	    VkDescriptorType type)
    {
	    //Allowed image types:
	    //VK_DESCRIPTOR_TYPE_SAMPLER
	    //VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
	    //VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE
	    //VK_DESCRIPTOR_TYPE_STORAGE_IMAGE
	    //VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT

	    VkDescriptorImageInfo imageInfo{ sampler, image, layout };
	    const VkDescriptorImageInfo& info = m_ImageInfos.emplace_back(imageInfo);

	    VkWriteDescriptorSet write{};
	    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	    write.dstBinding = binding;
	    write.dstSet = VK_NULL_HANDLE; //left empty for now until we need to write it
	    write.descriptorCount = 1;
	    write.descriptorType = type;
	    write.pImageInfo = &info;

	    m_Writes.push_back(write);
    }

	void DescriptorWriter::WriteBuffer(int binding, VkBuffer buffer, size_t size, size_t offset, VkDescriptorType type)
	{
		//Allowed buffer types:
		//VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER
		//VK_DESCRIPTOR_TYPE_STORAGE_BUFFER
		//VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC
		//VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC

		VkDescriptorBufferInfo bufferInfo{buffer, offset, size };
		const VkDescriptorBufferInfo& info = m_BufferInfos.emplace_back(bufferInfo);

		VkWriteDescriptorSet write{};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.dstBinding = binding;
		write.dstSet = VK_NULL_HANDLE; //left empty for now until we need to write it
		write.descriptorCount = 1;
		write.descriptorType = type;
		write.pBufferInfo = &info;

		m_Writes.push_back(write);
	}

	void DescriptorWriter::UpdateSet(VkDevice device, VkDescriptorSet set)
	{
		for (VkWriteDescriptorSet& write : m_Writes)
		{
			write.dstSet = set;
		}

		vkUpdateDescriptorSets(device, static_cast<uint32_t>(m_Writes.size()), m_Writes.data(), 0, nullptr);
	}

	void DescriptorWriter::Cleanup()
	{
		m_Writes.clear();
		m_ImageInfos.clear();
		m_BufferInfos.clear();
	}


	

	//	_____                     _       _               ____        _ _     _           
	// |  __ \                   (_)     | |             |  _ \      (_| |   | |          
	// | |  | | ___ ___  ___ _ __ _ _ __ | |_ ___  _ __  | |_) |_   _ _| | __| | ___ _ __ 
	// | |  | |/ _ / __|/ __| '__| | '_ \| __/ _ \| '__| |  _ <| | | | | |/ _` |/ _ | '__|
	// | |__| |  __\__ | (__| |  | | |_) | || (_) | |    | |_) | |_| | | | (_| |  __| |   
	// |_____/ \___|___/\___|_|  |_| .__/ \__\___/|_|    |____/ \__,_|_|_|\__,_|\___|_|   
	//                             | |                                                    
	//                             |_|                                                    

	VkDescriptorSetLayout DescriptorBuilder::Build(VkDevice device, VkShaderStageFlags shaderStages)
	{
		for (auto& binding : m_Bindings) 
		{
			binding.stageFlags |= shaderStages;
		}

		VkDescriptorSetLayoutCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		info.pNext = nullptr;
		info.pBindings = m_Bindings.data();
		info.bindingCount = static_cast<uint32_t>(m_Bindings.size());
		info.flags = 0;

		VkDescriptorSetLayout set;
		VulkanCheck(vkCreateDescriptorSetLayout(device, &info, nullptr, &set), "Failed To Create Descriptor Set Layout");

		return set;
	}

	void DescriptorBuilder::AddBinding(uint32_t binding, VkDescriptorType type)
	{
		VkDescriptorSetLayoutBinding layoutBinding{};
		layoutBinding.binding = binding;
		layoutBinding.descriptorType = type;
		layoutBinding.descriptorCount = 1;
		layoutBinding.stageFlags = VK_SHADER_STAGE_ALL;
		layoutBinding.pImmutableSamplers = nullptr;

		m_Bindings.push_back(layoutBinding);
	}

	void DescriptorBuilder::Cleanup()
	{
		m_Bindings.clear();
	}

	//_____                     _       _               __  __                                   
	//|  __ \                   (_)     | |             |  \/  |                                  
	//| |  | | ___ ___  ___ _ __ _ _ __ | |_ ___  _ __  | \  / | __ _ _ __   __ _  __ _  ___ _ __ 
	//| |  | |/ _ / __|/ __| '__| | '_ \| __/ _ \| '__| | |\/| |/ _` | '_ \ / _` |/ _` |/ _ | '__|
	//| |__| |  __\__ | (__| |  | | |_) | || (_) | |    | |  | | (_| | | | | (_| | (_| |  __| |   
	//|_____/ \___|___/\___|_|  |_| .__/ \__\___/|_|    |_|  |_|\__,_|_| |_|\__,_|\__, |\___|_|   
	//							  | |                                              __/ |          
	//							  |_|                                             |___/           



	void DescriptorManager::Init(VulkanContext* vulkanContext)
	{
		//for (int i{}; i < SwapChain::ImageCount(); ++i)
		{
			// create a descriptor pool
			std::vector<DescriptorAllocator::PoolSizeRatio> frame_sizes =
			{
				{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 3 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 3 },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3 },
				{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4 },
			};


			std::unique_ptr<DescriptorAllocator> allocator = std::make_unique<DescriptorAllocator>();
			allocator->Init(vulkanContext->device, 1000, frame_sizes);
			m_FrameAllocators.emplace_back(std::move(allocator));
		}

		GlobalDescriptor::Init(vulkanContext);
	}

	void DescriptorManager::Cleanup(VkDevice device)
	{
		GlobalDescriptor::Cleanup(device);

		for (const auto& allocator : m_FrameAllocators)
		{
			allocator->Cleanup(device);
		}
	}



	VkDescriptorSet DescriptorManager::Allocate(VkDevice device, VkDescriptorSetLayout setLayout, uint8_t frameNumber)
	{
		return m_FrameAllocators[frameNumber]->Allocate(device, setLayout);
	}

	void DescriptorManager::ClearPools(VkDevice device, uint8_t frameNumber)
	{
		m_FrameAllocators[0]->ClearPools(device);
	}

}
