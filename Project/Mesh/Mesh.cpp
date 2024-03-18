#include "Mesh.h"
#include <stdexcept>
#include "vulkanbase/VulkanUtil.h"
#include "vulkanbase/VulkanTypes.h"
#include "Vertex.h"
#include "Core/Buffer.h"
#include "Core/Logger.h"
#include "Patterns/ServiceLocator.h"

Mesh::Mesh(const std::vector<Vertex>& vertices)
{
	m_pContext = ServiceLocator::GetService<VulkanContext>();
	CreateVertexBuffer(vertices);
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
	vkDestroyBuffer(m_pContext->device, m_VertexBuffer, nullptr);
	vkFreeMemory(m_pContext->device, m_VertexBufferMemory, nullptr);
}

void Mesh::CreateVertexBuffer(const std::vector<Vertex>& vertices)
{
	//Store the vertex count
	m_VertexCount = static_cast<uint32_t>(vertices.size());

	//Check if the mesh has at least 3 vertices
	LogAssert(m_VertexCount >= 3,"Mesh has less then 3 vertices", false);

	//Get the device and the buffer size
	const VkDevice device = m_pContext->device;
	const VkDeviceSize bufferSize = sizeof(vertices[0]) * m_VertexCount;

	//Create a staging buffer
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	Core::Buffer::CreateStagingBuffer(m_pContext, bufferSize, vertices, stagingBuffer, stagingBufferMemory);


	//Create a Vertex buffer
	Core::Buffer::CreateBuffer(m_pContext, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_VertexBuffer, m_VertexBufferMemory);

	//Copy the staging buffer to the vertex buffer
	Core::Buffer::CopyBuffer(m_pContext, stagingBuffer, m_VertexBuffer, bufferSize);


	//Clean up the staging buffer
	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);
}
