#pragma once
#include <vulkan/vulkan.h>
#include <glm/mat4x4.hpp>
#include <unordered_map>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "Buffer.h"
#include "Camera/Camera.h"
#include "Core/Logger.h"
#include "shaders/Shader.h"

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
	private:

		std::vector<VkWriteDescriptorSet> m_Writes;
		std::vector<VkDescriptorSetLayoutBinding> m_Bindings;


		DescriptorLayoutCache* m_Cache;
		DescriptorAllocator* m_Allocator;
	};

	VkDescriptorPool CreatePool(VkDevice device, const DescriptorAllocator::PoolSizes& poolSizes, int maxSets, VkDescriptorPoolCreateFlags flags = 0);


	struct GlobalDescriptor
	{
		inline void Init(VulkanContext* vulkanContext)
		{
			m_GlobalBuffer.AddVariable(Camera::GetViewProjectionMatrix());
			//projectionHandle = m_GlobalBuffer.AddVariable(Camera::GetProjectionMatrix());
			//viewProjectionHandle = m_GlobalBuffer.AddVariable(Camera::GetViewProjectionMatrix());
			cameraHandle = m_GlobalBuffer.AddVariable(glm::vec4(Camera::GetPosition(), 1.0f));
			m_GlobalBuffer.BindBuffer(vulkanContext, 0, VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT, m_GlobalDescriptorSet, m_GlobalDescriptorSetLayout);
		}

		inline void Bind(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout)
		{
			m_GlobalBuffer.UpdateVariable(0, Camera::Camera::GetViewProjectionMatrix());
			//m_GlobalBuffer.UpdateVariable(projectionHandle, Camera::Camera::GetProjectionMatrix());
			//m_GlobalBuffer.UpdateVariable(viewProjectionHandle, Camera::Camera::GetViewProjectionMatrix());
			m_GlobalBuffer.UpdateVariable(cameraHandle, glm::vec4(Camera::GetPosition(), 1.0f));
			m_GlobalBuffer.Bind(commandBuffer, pipelineLayout, m_GlobalDescriptorSet);
		}

		VkDescriptorSetLayout& GetLayout() { return m_GlobalDescriptorSetLayout; }

		inline void Cleanup(VkDevice device)
		{
			m_GlobalBuffer.Cleanup(device);
		}

	private:
		VkDescriptorSetLayout m_GlobalDescriptorSetLayout{};
		VkDescriptorSet m_GlobalDescriptorSet{};
		DynamicBuffer m_GlobalBuffer{};

		uint16_t projectionHandle = 0;
		uint16_t viewProjectionHandle = 0;
		uint16_t cameraHandle = 0;
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

		static void Init(VulkanContext* vulkanContext)
		{
			m_pDescriptorAllocator->Init(vulkanContext->device);
			m_pDescriptorCache->Init(vulkanContext->device);
			m_DescriptorBuilder = DescriptorBuilder::Begin(m_pDescriptorCache.get(), m_pDescriptorAllocator.get());

			m_GlobalDescriptor.Init(vulkanContext);
		}

		static void Cleanup()
		{
			m_GlobalDescriptor.Cleanup(m_pDescriptorAllocator->m_Device);
			m_pDescriptorAllocator->Cleanup();
			m_pDescriptorCache->Cleanup();
		}

		static DescriptorAllocator* GetAllocator() { return m_pDescriptorAllocator.get(); }
		static DescriptorLayoutCache* GetCache() { return m_pDescriptorCache.get(); }
		static DescriptorBuilder& GetBuilder() { return m_DescriptorBuilder; }


		inline static GlobalDescriptor m_GlobalDescriptor;

	private:
		static inline std::unique_ptr<DescriptorAllocator> m_pDescriptorAllocator = std::make_unique<DescriptorAllocator>();
		static inline std::unique_ptr<DescriptorLayoutCache> m_pDescriptorCache = std::make_unique<DescriptorLayoutCache>();
		static inline DescriptorBuilder m_DescriptorBuilder;
	};
}
