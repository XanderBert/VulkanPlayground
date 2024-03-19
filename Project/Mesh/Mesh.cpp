#include "Mesh.h"
#include <stdexcept>
#include "vulkanbase/VulkanUtil.h"
#include "vulkanbase/VulkanTypes.h"
#include "Vertex.h"
#include "Core/Buffer.h"
#include "Core/Logger.h"
#include "Patterns/ServiceLocator.h"

Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<uint16_t>& indices)
{
	m_pContext = ServiceLocator::GetService<VulkanContext>();
	CreateVertexBuffer(vertices);
	CreateIndexBuffer(indices);
}

void Mesh::Bind(VkCommandBuffer commandBuffer) const
{
	const VkBuffer vertexBuffers[] = { m_VertexBuffer };
	constexpr VkDeviceSize offsets[] = { 0 };

	vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

	vkCmdBindIndexBuffer(commandBuffer, m_IndexBuffer, 0, VK_INDEX_TYPE_UINT16);
}


void Mesh::Render(VkCommandBuffer commandBuffer) const
{
	vkCmdDrawIndexed(commandBuffer, m_IndexCount, 1, 0, 0, 0);
}

void Mesh::CleanUp() const
{
	vkDestroyBuffer(m_pContext->device, m_VertexBuffer, nullptr);
	vkFreeMemory(m_pContext->device, m_VertexBufferMemory, nullptr);

	vkDestroyBuffer(m_pContext->device, m_IndexBuffer, nullptr);
	vkFreeMemory(m_pContext->device, m_IndexBufferMemory, nullptr);
}

void Mesh::CreateVertexBuffer(const std::vector<Vertex>& vertices)
{
	//Store the vertex count
	m_VertexCount = static_cast<uint16_t>(vertices.size());

	//Check if the mesh has at least 3 vertices
	LogAssert(m_VertexCount >= 3,"Mesh has less then 3 vertices", false);

	//Get the device and the buffer size
	const VkDevice device = m_pContext->device;
	const VkDeviceSize bufferSize = sizeof(vertices[0]) * m_VertexCount;

	//Create a staging buffer
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;


	Core::Buffer::CreateStagingBuffer<Vertex>(m_pContext, bufferSize, stagingBuffer, stagingBufferMemory, vertices.data());


	//Create a Vertex buffer
	Core::Buffer::CreateBuffer(m_pContext, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_VertexBuffer, m_VertexBufferMemory);

	//Copy the staging buffer to the vertex buffer
	Core::Buffer::CopyBuffer(m_pContext, stagingBuffer, m_VertexBuffer, bufferSize);


	//Clean up the staging buffer
	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void Mesh::CreateIndexBuffer(const std::vector<uint16_t>& indices)
{
	m_IndexCount = static_cast<uint16_t>(indices.size());
	LogAssert(m_IndexCount >= 3, "Mesh has less then 3 indices", false);


	//Get the device and the buffer size
	const VkDevice device = m_pContext->device;
	const VkDeviceSize bufferSize = sizeof(indices[0]) * m_IndexCount;

	auto data = indices.data();

	//Create a staging buffer
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	Core::Buffer::CreateStagingBuffer<uint16_t>(m_pContext, bufferSize, stagingBuffer, stagingBufferMemory, indices.data());


	//Create A index buffer
	Core::Buffer::CreateBuffer(m_pContext, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_IndexBuffer, m_IndexBufferMemory);

	//Copy the staging buffer to the index buffer
	Core::Buffer::CopyBuffer(m_pContext, stagingBuffer, m_IndexBuffer, bufferSize);

	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);
}
