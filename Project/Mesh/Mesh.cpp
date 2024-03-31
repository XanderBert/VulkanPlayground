#include "Mesh.h"
#include <chrono>

#include "../vulkanbase/VulkanTypes.h"
#include "Vertex.h"
#include "Patterns/ServiceLocator.h"


Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<uint16_t>& indices)
{
	m_pContext = ServiceLocator::GetService<VulkanContext>();
	CreateVertexBuffer(vertices);
	CreateIndexBuffer(indices);

	m_pMaterial = std::make_unique<Material>(m_pContext);
	m_pMaterial->AddShader("shader.vert", ShaderType::VertexShader);
	m_pMaterial->AddShader("shader.frag", ShaderType::FragmentShader);


	float time = 1.1f;
	glm::mat4x4 model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	glm::mat4x4 view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	glm::mat4x4 proj = glm::perspective(glm::radians(45.0f), m_pContext->swapChainExtent.width / (float)m_pContext->swapChainExtent.height, 0.1f, 10.0f);
	proj[1][1] *= -1;


	m_VariableHandles.push_back(m_pMaterial->AddShaderVariable(model));
	m_VariableHandles.push_back(m_pMaterial->AddShaderVariable(view));
	m_VariableHandles.push_back(m_pMaterial->AddShaderVariable(proj));

	m_pMaterial->CreatePipeline();
}

void Mesh::Bind(VkCommandBuffer commandBuffer) const
{
	static auto startTime = std::chrono::high_resolution_clock::now();
	auto currentTime = std::chrono::high_resolution_clock::now();
	float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

	glm::mat4x4 model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	glm::mat4x4 view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	glm::mat4x4 proj = glm::perspective(glm::radians(45.0f), m_pContext->swapChainExtent.width / (float)m_pContext->swapChainExtent.height, 0.1f, 10.0f);
	proj[1][1] *= -1;

	m_pMaterial->UpdateShaderVariable(m_VariableHandles[0], model);
	m_pMaterial->UpdateShaderVariable(m_VariableHandles[1], view);
	m_pMaterial->UpdateShaderVariable(m_VariableHandles[2], proj);

	m_pMaterial->Bind(commandBuffer);


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
	m_pMaterial->CleanUp();

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