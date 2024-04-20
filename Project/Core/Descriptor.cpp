#include "Descriptor.h"
#include <algorithm>

namespace Descriptor
{
	VkDescriptorPool CreatePool(VkDevice device, const DescriptorAllocator::PoolSizes& poolSizes, int maxSets, VkDescriptorPoolCreateFlags flags)
	{
		//Create pool sizes
		std::vector<VkDescriptorPoolSize> sizes;
		sizes.reserve(poolSizes.sizes.size());


		for (auto& sz : poolSizes.sizes) 
		{
			sizes.push_back({ sz.first, static_cast<uint32_t>(maxSets * sz.second) });
		}


		VkDescriptorPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.flags = flags;
		poolInfo.maxSets = maxSets;
		poolInfo.poolSizeCount = static_cast<uint32_t>(sizes.size());
		poolInfo.pPoolSizes = sizes.data();

		VkDescriptorPool descriptorPool;
		VulkanCheck(vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool), "Failed To Create A Descriptor Pool")

		return descriptorPool;
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
		m_pDescriptorAllocator->Init(vulkanContext->device);
		m_pDescriptorCache->Init(vulkanContext->device);
		m_DescriptorBuilder = DescriptorBuilder::Begin(m_pDescriptorCache.get(), m_pDescriptorAllocator.get());

		m_GlobalDescriptor.Init(vulkanContext);
	}

	void DescriptorManager::Cleanup()
	{
		m_GlobalDescriptor.Cleanup(m_pDescriptorAllocator->m_Device);
		m_pDescriptorAllocator->Cleanup();
		m_pDescriptorCache->Cleanup();
	}

	DescriptorAllocator* DescriptorManager::GetAllocator()
	{
		return m_pDescriptorAllocator.get();
	}

	DescriptorLayoutCache* DescriptorManager::GetCache()
	{
		return m_pDescriptorCache.get();
	}

	DescriptorBuilder& DescriptorManager::GetBuilder()
	{
		return m_DescriptorBuilder;
	}

	// _____                     _       _                        _ _                 _             
	//|  __ \                   (_)     | |                 /\   | | |               | |            
	//| |  | | ___ ___  ___ _ __ _ _ __ | |_ ___  _ __     /  \  | | | ___   ___ __ _| |_ ___  _ __ 
	//| |  | |/ _ / __|/ __| '__| | '_ \| __/ _ \| '__|   / /\ \ | | |/ _ \ / __/ _` | __/ _ \| '__|
	//| |__| |  __\__ | (__| |  | | |_) | || (_) | |     / ____ \| | | (_) | (_| (_| | || (_) | |   
	//|_____/ \___|___/\___|_|  |_| .__/ \__\___/|_|    /_/    \_|_|_|\___/ \___\__,_|\__\___/|_|   
    //                          | |                                                               
    //                          |_|                                                               
	//


	void DescriptorAllocator::Init(VkDevice device)
	{
		m_Device = device;
	}

	void DescriptorAllocator::ResetPools()
	{
		for(const VkDescriptorPool usedPool : m_UsedPools)
		{
			vkResetDescriptorPool(m_Device, usedPool, 0);
		}

		m_FreePools = m_UsedPools;
		m_UsedPools.clear();
		m_CurrentPool = VK_NULL_HANDLE;
	}

	bool DescriptorAllocator::Allocate(VkDescriptorSet* set, VkDescriptorSetLayout layout)
	{
		if (m_CurrentPool == VK_NULL_HANDLE)
		{
			m_CurrentPool = GrabPool();
			m_UsedPools.push_back(m_CurrentPool);
		}

		VkDescriptorSetAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.pSetLayouts = &layout;
		allocInfo.descriptorPool = m_CurrentPool;
		allocInfo.descriptorSetCount = 1;


		const VkResult allocateResult = vkAllocateDescriptorSets(m_Device, &allocInfo, set);

		if (allocateResult == VK_ERROR_FRAGMENTED_POOL || allocateResult == VK_ERROR_OUT_OF_POOL_MEMORY)
		{
			//reallocate pool
			m_CurrentPool = GrabPool();
			m_UsedPools.push_back(m_CurrentPool);

			allocInfo.descriptorPool = m_CurrentPool;

			VulkanCheck(vkAllocateDescriptorSets(m_Device, &allocInfo, set), "Failed To AllocateDescriptor Sets");
		}

		return true;
	}

	void DescriptorAllocator::Cleanup() const
	{
		for (const VkDescriptorPool usedPool : m_UsedPools)
		{
			vkDestroyDescriptorPool(m_Device, usedPool, nullptr);
		}

		for (const VkDescriptorPool pool : m_FreePools)
		{
			vkDestroyDescriptorPool(m_Device, pool, nullptr);
		}
	}

	VkDescriptorPool DescriptorAllocator::GrabPool()
	{
		if (m_FreePools.empty())
		{
			return CreatePool(m_Device, m_DescriptorSizes, 1000, 0);
		}

		const VkDescriptorPool pool = m_FreePools.back();
		m_FreePools.pop_back();
		return pool;
	}

	// _____                     _       _               _                             _      _____           _          
	//|  __ \                   (_)     | |             | |                           | |    / ____|         | |         
	//| |  | | ___ ___  ___ _ __ _ _ __ | |_ ___  _ __  | |     __ _ _   _  ___  _   _| |_  | |     __ _  ___| |__   ___ 
	//| |  | |/ _ / __|/ __| '__| | '_ \| __/ _ \| '__| | |    / _` | | | |/ _ \| | | | __| | |    / _` |/ __| '_ \ / _ \
	//| |__| |  __\__ | (__| |  | | |_) | || (_) | |    | |___| (_| | |_| | (_) | |_| | |_  | |___| (_| | (__| | | |  __/
	//|_____/ \___|___/\___|_|  |_| .__/ \__\___/|_|    |______\__,_|\__, |\___/ \__,_|\__|  \_____\__,_|\___|_| |_|\___|
	//                            | |                                 __/ |                                              
	//                            |_|                                |___/                                               
	//


	void DescriptorLayoutCache::Init(VkDevice device)
	{
		m_Device = device;
	}

	void DescriptorLayoutCache::Cleanup() const
	{
		for (const auto& layout : m_LayoutCache)
		{
			vkDestroyDescriptorSetLayout(m_Device, layout.second, nullptr);
		}
	}

	VkDescriptorSetLayout DescriptorLayoutCache::CreateDescriptorLayout(const VkDescriptorSetLayoutCreateInfo* info)
	{
		DescriptorLayoutInfo layoutInfo;

		layoutInfo.bindings.reserve(info->bindingCount);
		bool isSorted = true;
		uint32_t lastBinding = -1;
		
		
		for (uint32_t i{}; i < info->bindingCount; ++i)
		{
			//Copy the info struct 
			layoutInfo.bindings.push_back(info->pBindings[i]);



			//check that the bindings are in strict increasing order
			if(!isSorted) continue;

			if (info->pBindings[i].binding > lastBinding)
			{
				lastBinding = info->pBindings[i].binding;
				continue;
			}
			
			isSorted = false;
		}

		//Sort the bindings if they are not so
		if (!isSorted)
		{
			std::sort(layoutInfo.bindings.begin(), layoutInfo.bindings.end(), [](const VkDescriptorSetLayoutBinding& a, const VkDescriptorSetLayoutBinding& b) 
			{ return a.binding < b.binding; });
		}


		//Grab from cache if it exists
		const auto it = m_LayoutCache.find(layoutInfo);
		if (it != m_LayoutCache.end()) return it->second;


		//Create a new one if its not in the cache
		VkDescriptorSetLayout layout;
		VulkanCheck(vkCreateDescriptorSetLayout(m_Device, info, nullptr, &layout), "Failed to Create Descriptor Set Layout");

		//Store in cache
		m_LayoutCache[layoutInfo] = layout;
		return layout;
	}

	bool DescriptorLayoutCache::DescriptorLayoutInfo::operator==(const DescriptorLayoutInfo& other) const
	{
		if (bindings.size() != other.bindings.size()) return false;


		//Check if all the (sorted) bindings are the same.
		for (uint32_t i{}; i < bindings.size(); ++i)
		{
			if (bindings[i].binding != other.bindings[i].binding) return false;
			if (bindings[i].descriptorCount != other.bindings[i].descriptorCount) return false;
			if (bindings[i].descriptorType != other.bindings[i].descriptorType) return false;
			if (bindings[i].stageFlags != other.bindings[i].stageFlags) return false;
		}

		return true;
	}

	size_t DescriptorLayoutCache::DescriptorLayoutInfo::hash() const
	{
		//Create a hash from the bindings

		size_t hash = 0;
		for (const VkDescriptorSetLayoutBinding& binding : bindings)
		{
			constexpr unsigned int phi = 0x9e3779b9; //(golden ratio)

			hash ^= (phi + binding.binding + (hash << 6) + (hash >> 2)) ^
				(phi + binding.descriptorCount + (hash << 6) + (hash >> 2)) ^
				(phi + binding.descriptorType + (hash << 6) + (hash >> 2)) ^
				(phi + binding.stageFlags + (hash << 6) + (hash >> 2));
		}

		return hash;
	}

	std::size_t DescriptorLayoutCache::DescriptorLayoutHash::operator()(const DescriptorLayoutInfo& layoutInfo) const
	{
		return layoutInfo.hash();
	}

	//	_____                     _       _               ____        _ _     _           
	// |  __ \                   (_)     | |             |  _ \      (_| |   | |          
	// | |  | | ___ ___  ___ _ __ _ _ __ | |_ ___  _ __  | |_) |_   _ _| | __| | ___ _ __ 
	// | |  | |/ _ / __|/ __| '__| | '_ \| __/ _ \| '__| |  _ <| | | | | |/ _` |/ _ | '__|
	// | |__| |  __\__ | (__| |  | | |_) | || (_) | |    | |_) | |_| | | | (_| |  __| |   
	// |_____/ \___|___/\___|_|  |_| .__/ \__\___/|_|    |____/ \__,_|_|_|\__,_|\___|_|   
	//                             | |                                                    
	//                             |_|                                                    

	DescriptorBuilder DescriptorBuilder::Begin(DescriptorLayoutCache* layoutCache, DescriptorAllocator* allocator)
	{
		DescriptorBuilder builder;

		builder.m_Cache = layoutCache;
		builder.m_Allocator = allocator;

		return builder;
	}

	DescriptorBuilder& DescriptorBuilder::BindBuffer(uint32_t binding, const VkDescriptorBufferInfo* bufferInfo, VkDescriptorType type, VkShaderStageFlags stageFlags)
	{
		//create the descriptor binding for the layout
		VkDescriptorSetLayoutBinding newBinding{};

		newBinding.descriptorCount = 1;
		newBinding.descriptorType = type;
		newBinding.pImmutableSamplers = nullptr;
		newBinding.stageFlags = stageFlags;
		newBinding.binding = binding;

		m_Bindings.push_back(newBinding);


		//create the descriptor write
		VkWriteDescriptorSet newWrite{};
		newWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		newWrite.descriptorCount = 1;
		newWrite.descriptorType = type;
		newWrite.pBufferInfo = bufferInfo;
		newWrite.dstBinding = binding;
		m_Writes.push_back(newWrite);


		return *this;
	}

	DescriptorBuilder& DescriptorBuilder::BindImage(uint32_t binding, const VkDescriptorImageInfo* imageInfo, VkDescriptorType type, VkShaderStageFlags stageFlags)
	{
		//create the descriptor binding for the layout
		VkDescriptorSetLayoutBinding newBinding{};

		newBinding.descriptorCount = 1;
		newBinding.descriptorType = type;
		newBinding.pImmutableSamplers = nullptr;
		newBinding.stageFlags = stageFlags;
		newBinding.binding = binding;
		

		m_Bindings.push_back(newBinding);

		//create the descriptor write for a image
		VkWriteDescriptorSet newWrite{};
		newWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		newWrite.pNext = nullptr;
		newWrite.descriptorCount = 1;
		newWrite.descriptorType = type;
		newWrite.pImageInfo = imageInfo;
		newWrite.dstBinding = binding;

		m_Writes.push_back(newWrite);


		return *this;
	}

	bool DescriptorBuilder::Build(VkDescriptorSet& set, VkDescriptorSetLayout& layout)
	{
		//Create the descriptor set layout
		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(m_Bindings.size());
		layoutInfo.pBindings = m_Bindings.data();

		layout = m_Cache->CreateDescriptorLayout(&layoutInfo);

		//Allocate the descriptor set
		if (!m_Allocator->Allocate(&set, layout))
		{
			LogError("Failed to allocate the descriptor set");
			return false;
		}

		//Set the write descriptor set
		for (VkWriteDescriptorSet& write : m_Writes)
		{
			write.dstSet = set;
		}
		
		vkUpdateDescriptorSets(m_Allocator->m_Device, static_cast<uint32_t>(m_Writes.size()), m_Writes.data(), 0, nullptr);

		return true;
	}
}