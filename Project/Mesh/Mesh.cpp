#include "Mesh.h"
#include <stdexcept>
#include "vulkanbase/VulkanUtil.h"
#include "vulkanbase/VulkanTypes.h"
#include "Vertex.h"
#include "Patterns/ServiceLocator.h"

Mesh::Mesh(const std::vector<Vertex>& vertices)
{
	const VulkanContext* context = ServiceLocator::GetService<VulkanContext>();
	m_pDevice = context->device;


	CreateVertexBuffer(vertices, context->physicalDevice);
}

void Mesh::Bind(VkCommandBuffer commandBuffer) const
{
	const VkBuffer vertexBuffers[] = { m_VertexBuffer };
	constexpr VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
}


void Mesh::Render(VkCommandBuffer commandBuffer) const
{
	vkCmdDraw(commandBuffer, m_VertexCount, 1, 0, 0);
}

void Mesh::CleanUp() const
{
	vkDestroyBuffer(m_pDevice, m_VertexBuffer, nullptr);
	vkFreeMemory(m_pDevice, m_VertexBufferMemory, nullptr);
}

void Mesh::CreateVertexBuffer(const std::vector<Vertex>& vertices, VkPhysicalDevice physicalDevice)
{
	m_VertexCount = static_cast<uint32_t>(vertices.size());
	assert(m_VertexCount >= 3);

	//Create a buffer
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = sizeof(Vertex) * m_VertexCount;
	bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;


	if (vkCreateBuffer(m_pDevice, &bufferInfo, nullptr, &m_VertexBuffer) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create vertex buffer!");
	}


	//Assign memory to the buffer
	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(m_pDevice, m_VertexBuffer, &memRequirements);

	//Allocate memory
	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = tools::findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, physicalDevice);

	if (vkAllocateMemory(m_pDevice, &allocInfo, nullptr, &m_VertexBufferMemory) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate vertex buffer memory!");
	}


	vkBindBufferMemory(m_pDevice, m_VertexBuffer, m_VertexBufferMemory, 0);


	void* data;
	vkMapMemory(m_pDevice, m_VertexBufferMemory, 0, bufferInfo.size, 0, &data);
	//Copy the actual data into the buffer
	memcpy(data, vertices.data(), (size_t)bufferInfo.size);
	vkUnmapMemory(m_pDevice, m_VertexBufferMemory);
}
