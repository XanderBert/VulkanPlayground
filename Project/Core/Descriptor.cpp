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
			m_PoolSizeRatios.emplace_back(PoolSizeRatio{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1.0f });
			m_PoolSizeRatios.emplace_back(PoolSizeRatio{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1.0f });
		}
		else
		{
			m_PoolSizeRatios = poolRatios;
		}


		VkDescriptorPool newPool = CreatePool(device, maxSets, poolRatios);

		setsPerPool = static_cast<uint32_t>(1.5f * static_cast<float>(maxSets)); //grow it next allocation

		m_ReadyPools.emplace_back(newPool);
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
			m_ReadyPools.emplace_back(pool);
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

			m_FullPools.emplace_back(poolToUse);

			poolToUse = GrabPool(device);
			allocInfo.descriptorPool = poolToUse;

			VulkanCheck(vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet), "Failed To Allocate Descriptor Set!")
		}

		m_ReadyPools.emplace_back(poolToUse);
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
		constexpr int maxSets = 4096; //(4 * 1024) - ave. usage
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

			setsPerPool = static_cast<uint32_t>(static_cast<float>(setsPerPool) * setMultiplier);
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
			poolSize.descriptorCount = static_cast<uint32_t>(ratio.ratio * static_cast<float>(setCount));

			poolSizes.emplace_back(poolSize);
		}

		VkDescriptorPoolCreateInfo pool_info = {};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT_EXT; //This flag is needed to allow the creation of descriptor sets that can be updated after they have been bound.
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

    void DescriptorWriter::WriteImage(DescriptorResourceHandle resourceHandle, VkImageView image, VkSampler sampler, VkImageLayout layout, DescriptorType type)
    {
	    //Allowed image types:
	    //VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
	    //VK_DESCRIPTOR_TYPE_STORAGE_IMAGE

	    const VkDescriptorImageInfo& info = m_ImageInfos.emplace_back(VkDescriptorImageInfo{ sampler, image, layout });

	    VkWriteDescriptorSet write{};
	    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;

		//Get the Correct Binding
		write.dstBinding = type == DescriptorType::CombinedImageSampler ? TextureBinding : StorageTextureBinding;

		//use the handle as array index
		write.dstArrayElement = static_cast<uint32_t>(resourceHandle);

	    write.dstSet = VK_NULL_HANDLE; //left empty for now until we actually write the set
	    write.descriptorCount = 1;
	    write.descriptorType = static_cast<VkDescriptorType>(type);
	    write.pImageInfo = &info;

	    m_Writes.emplace_back(write);
    }

	void DescriptorWriter::WriteBuffer(DescriptorResourceHandle resourceHandle, VkBuffer buffer, size_t size, size_t offset, DescriptorType type)
	{
		//Allowed buffer types:
		//VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER
		//VK_DESCRIPTOR_TYPE_STORAGE_BUFFER

		const VkDescriptorBufferInfo& info = m_BufferInfos.emplace_back(VkDescriptorBufferInfo{buffer, offset, size });

		VkWriteDescriptorSet write{};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;

		//Get the Correct Binding
		write.dstBinding = type == DescriptorType::UniformBuffer ? UniformBufferBinding : StorageBufferBinding;

		//use the handle as array index
		write.dstArrayElement = static_cast<uint32_t>(resourceHandle);

		write.dstSet = VK_NULL_HANDLE; //left empty for now until we actually write the set
		write.descriptorCount = 1;
		write.descriptorType = static_cast<VkDescriptorType>(type);
		write.pBufferInfo = &info;

		m_Writes.emplace_back(write);
	}

	void DescriptorWriter::UpdateSet(VkDevice device, VkDescriptorSet set)
	{
		//TODO: Actual update the write
		//For now we just set the dst
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

	DescriptorBuilder::DescriptorBuilder()
	{
		//Setup the bindings
		VkDescriptorSetLayoutBinding& image_sampler_binding = m_Bindings[ 0 ];
		image_sampler_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		image_sampler_binding.descriptorCount = MaxBindlessResources;
		image_sampler_binding.binding = TextureBinding;

		VkDescriptorSetLayoutBinding& storage_image_binding = m_Bindings[ 1 ];
		storage_image_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		storage_image_binding.descriptorCount = MaxBindlessResources;
		storage_image_binding.binding = StorageTextureBinding;

		VkDescriptorSetLayoutBinding& uniform_buffer_binding = m_Bindings[ 2 ];
		uniform_buffer_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uniform_buffer_binding.descriptorCount = MaxBindlessResources;
		uniform_buffer_binding.binding = UniformBufferBinding;

		VkDescriptorSetLayoutBinding& storage_buffer_binding = m_Bindings[ 3 ];
		storage_buffer_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		storage_buffer_binding.descriptorCount = MaxBindlessResources;
		storage_buffer_binding.binding = StorageBufferBinding ;
	}


	void DescriptorBuilder::Build(VkDevice device, VkDescriptorSetLayout& descriptorSetLayout)
	{
		for (auto& binding : m_Bindings)
		{
			//binding.stageFlags |= shaderStages;
			binding.stageFlags = VK_SHADER_STAGE_ALL;
		}

		VkDescriptorSetLayoutCreateInfo layout_info ={ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
		layout_info.bindingCount = static_cast<uint32_t>(m_Bindings.size());
		layout_info.pBindings = m_Bindings.data();
		layout_info.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT;

		VkDescriptorBindingFlags bindless_flags = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT;
		VkDescriptorBindingFlags binding_flags[ 4 ];
		binding_flags[ 0 ] = bindless_flags;
		binding_flags[ 1 ] = bindless_flags;
		binding_flags[ 2 ] = bindless_flags;
		binding_flags[ 3 ] = bindless_flags;

		VkDescriptorSetLayoutBindingFlagsCreateInfoEXT extended_info{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO_EXT, nullptr };
		extended_info.bindingCount = static_cast<uint32_t>(m_Bindings.size());;
		extended_info.pBindingFlags = binding_flags;

		layout_info.pNext = &extended_info;
		VulkanCheck(vkCreateDescriptorSetLayout( device, &layout_info,nullptr, &descriptorSetLayout), "Failed To Create Descriptor Set Layout!");
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

			//Create a new allocator
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



	VkDescriptorSet DescriptorManager::Allocate(VkDevice device, VkDescriptorSetLayout setLayout, uint8_t frameNumber) {
        return m_FrameAllocators[frameNumber]->Allocate(device, setLayout);
    }

    void DescriptorManager::ClearPools(VkDevice device)
	{
		m_FrameAllocators[0]->ClearPools(device);
	}

}
