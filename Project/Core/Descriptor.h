#pragma once
#include <vulkan/vulkan.h>
#include <glm/mat4x4.hpp>
#include <unordered_map>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "Buffer.h"
#include "Core/Logger.h"

namespace Descriptor 
{
	//manages allocation of descriptor sets.
	//Will keep creating new descriptor pools once they get filled. it will reset the entire thing and reuse pools.
	class DescriptorAllocator final
	{
	public:
		DescriptorAllocator() = default;
		~DescriptorAllocator() = default;

		DescriptorAllocator(const DescriptorAllocator&) = delete;
		DescriptorAllocator& operator=(const DescriptorAllocator&) = delete;
		DescriptorAllocator(DescriptorAllocator&&) = delete;
		DescriptorAllocator& operator=(DescriptorAllocator&&) = delete;

		struct PoolSizes
		{
			//multiplier on the number of descriptor sets allocated for the pools.
			//These values should be played with to find the optimal value
			std::vector<std::pair<VkDescriptorType, float>> sizes =
			{
				{ VK_DESCRIPTOR_TYPE_SAMPLER, 0.5f },
				{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4.f },
				{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 4.f },
				{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1.f },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1.f },
				{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1.f },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2.f },
				{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2.f },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1.f },
				{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1.f },
				{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 0.5f }
			};
		};

		void Init(VkDevice device);
		void ResetPools();
		bool Allocate(VkDescriptorSet* set, VkDescriptorSetLayout layout);
		void Cleanup() const;

		VkDevice m_Device;


	private:
		VkDescriptorPool GrabPool();

		VkDescriptorPool m_CurrentPool{ VK_NULL_HANDLE };
		PoolSizes m_DescriptorSizes {};


		//Actively used pools
		std::vector<VkDescriptorPool> m_UsedPools;

		//Pools ready for reuse
		std::vector<VkDescriptorPool> m_FreePools;
	};


	//Caches for DescriptorSetLayouts to avoid creating duplicated layouts
	class DescriptorLayoutCache final
	{
	public:
		DescriptorLayoutCache() = default;
		~DescriptorLayoutCache() = default;

		DescriptorLayoutCache(const DescriptorLayoutCache&) = delete;
		DescriptorLayoutCache& operator=(const DescriptorLayoutCache&) = delete;
		DescriptorLayoutCache(DescriptorLayoutCache&&) = delete;
		DescriptorLayoutCache& operator=(DescriptorLayoutCache&&) = delete;

		void Init(VkDevice device);
		void Cleanup() const;

		VkDescriptorSetLayout CreateDescriptorLayout(const VkDescriptorSetLayoutCreateInfo* info);

		struct DescriptorLayoutInfo
		{
			std::vector<VkDescriptorSetLayoutBinding> bindings;

			//For Hash Checking
			bool operator==(const DescriptorLayoutInfo& other) const;
			size_t hash() const;
		};



	private:

		//Custom hash function for the DescriptorLayoutInfo
		struct DescriptorLayoutHash
		{
			std::size_t operator()(const DescriptorLayoutInfo& layoutInfo) const { return layoutInfo.hash(); }
		};

		std::unordered_map<DescriptorLayoutInfo, VkDescriptorSetLayout, DescriptorLayoutHash> m_LayoutCache;
		VkDevice m_Device;
	};


	//allocate and write a descriptor set and its layout automatically.
	class DescriptorBuilder final
	{
	public:
		DescriptorBuilder() = default;
		~DescriptorBuilder() = default;

		//DescriptorBuilder& operator=(const DescriptorBuilder&) = delete;
		//DescriptorBuilder& operator=(DescriptorBuilder&&) = delete;


		static DescriptorBuilder Begin(DescriptorLayoutCache* layoutCache, DescriptorAllocator* allocator);

		DescriptorBuilder& BindBuffer(uint32_t binding, const VkDescriptorBufferInfo* bufferInfo, VkDescriptorType type, VkShaderStageFlags stageFlags);
		DescriptorBuilder& BindImage(uint32_t binding, const VkDescriptorImageInfo* imageInfo, VkDescriptorType type, VkShaderStageFlags stageFlags);

		bool Build(VkDescriptorSet& set, VkDescriptorSetLayout& layout);
		//bool Build(VkDescriptorSet& set);
	private:

		std::vector<VkWriteDescriptorSet> m_Writes;
		std::vector<VkDescriptorSetLayoutBinding> m_Bindings;


		DescriptorLayoutCache* m_Cache;
		DescriptorAllocator* m_Allocator;
	};

	VkDescriptorPool CreatePool(VkDevice device, const DescriptorAllocator::PoolSizes& poolSizes, int maxSets, VkDescriptorPoolCreateFlags flags = 0);
}