#pragma once
#include <array>
#include <deque>
#include <memory>
#include <vector>
#include <vulkan/vulkan.h>
#include "Buffer.h"


enum class DescriptorResourceHandle : uint32_t;
enum class DescriptorType;

namespace Descriptor
{
	static constexpr uint32_t		 ParametersUboBinding = 1;
	static constexpr uint32_t		 UniformBufferBinding = 5;
	static constexpr uint32_t		 StorageBufferBinding = 6;
	static constexpr uint32_t        TextureBinding = 10;
	static constexpr uint32_t        StorageTextureBinding = 11;
	static constexpr uint32_t        MaxBindlessResources = 1024;

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

		struct PoolSizeRatio
		{
			VkDescriptorType type;
			float ratio;
		};

		//Allocate the first descriptor pool
		void Init(VkDevice device, uint32_t maxSets, const std::vector<PoolSizeRatio>& poolRatios = std::vector<PoolSizeRatio>{});

		//Copy the full pools to the ready pools and clear the full pools.
		void ClearPools(VkDevice device);

		//Allocate a descriptor set from the ready pools, if there are none, create a new pool.
		VkDescriptorSet Allocate(VkDevice device, VkDescriptorSetLayout layout);

		void Cleanup(VkDevice device);
	private:
		//Get a pool from the ready vector, if there are none, create a new one.
		VkDescriptorPool GrabPool(VkDevice device);

		static VkDescriptorPool CreatePool(VkDevice device, uint32_t setCount, const std::vector<PoolSizeRatio>& poolRatios);

		std::vector<PoolSizeRatio> m_PoolSizeRatios;
		std::vector<VkDescriptorPool> m_FullPools;
		std::vector<VkDescriptorPool> m_ReadyPools;
		uint32_t setsPerPool;
	};

	class DescriptorWriter final
	{
	public:
		DescriptorWriter() = default;
		~DescriptorWriter() = default;
		DescriptorWriter(const DescriptorWriter&) = delete;
		DescriptorWriter& operator=(const DescriptorWriter&) = delete;
		DescriptorWriter(DescriptorWriter&&) = delete;
		DescriptorWriter& operator=(DescriptorWriter&&) = delete;

		//Queue Image writes
		void WriteImage(DescriptorResourceHandle resourceHandle, VkImageView image, VkSampler sampler, VkImageLayout layout, DescriptorType type);

		//Queue Buffer writes
		void WriteBuffer(DescriptorResourceHandle resourceHandle, VkBuffer buffer, size_t size, size_t offset, DescriptorType type);

		//Actual Write to the descriptor set
		void UpdateSet(VkDevice device, VkDescriptorSet set);

		void Cleanup();

	private:

		//std::deque is guaranteed to keep pointers to elements valid
		std::deque<VkDescriptorImageInfo> m_ImageInfos;
		std::deque<VkDescriptorBufferInfo> m_BufferInfos;
		std::vector<VkWriteDescriptorSet> m_Writes;
	};


	//allocate and write a descriptor set and its layout automatically.
	class DescriptorBuilder final
	{
	public:
		DescriptorBuilder();
		~DescriptorBuilder() = default;

		DescriptorBuilder(const DescriptorBuilder&) = delete;
		DescriptorBuilder& operator=(const DescriptorBuilder&) = delete;
		DescriptorBuilder(DescriptorBuilder&&) = delete;
		DescriptorBuilder& operator=(DescriptorBuilder&&) = delete;

		void Build(VkDevice device, VkDescriptorSetLayout& descriptorSetLayout);

	private:
		std::array<VkDescriptorSetLayoutBinding, 4> m_Bindings{};
	};


	//Manages the above classes
	class DescriptorManager final
	{
	public:
		DescriptorManager() = default;
		~DescriptorManager() = default;

		DescriptorManager(const DescriptorManager&) = delete;
		DescriptorManager& operator=(const DescriptorManager&) = delete;
		DescriptorManager(DescriptorManager&&) = delete;
		DescriptorManager& operator=(DescriptorManager&&) = delete;

		static void Init(VulkanContext* vulkanContext);
		static void Cleanup(VkDevice device);


		static VkDescriptorSet Allocate(VkDevice device, VkDescriptorSetLayout setLayout);
		static void ClearPools(VkDevice device);

	private:
		static inline DescriptorAllocator m_FrameAllocator{};
	};
}
