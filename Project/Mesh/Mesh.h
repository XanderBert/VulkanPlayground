#pragma once
#include "vulkan/vulkan.h"
#include <vector>

class VulkanContext;
struct Vertex;
class Mesh final
{
public:
	Mesh(const std::vector<Vertex>& vertices, const std::vector<uint16_t>& indices);
	~Mesh() = default;

	Mesh(const Mesh&) = delete;
	Mesh(Mesh&&) = delete;
	Mesh& operator=(const Mesh&) = delete;
	Mesh& operator=(Mesh&&) = delete;

	void Bind(VkCommandBuffer commandBuffer) const;
	void Render(VkCommandBuffer commandBuffer) const;
	void CleanUp() const;

private:
	void CreateVertexBuffer(const std::vector<Vertex>& vertices);
	void CreateIndexBuffer(const std::vector<uint16_t>& indices);

	VulkanContext* m_pContext;


	//TODO: Store the vertex and index buffer in 1 VkBuffer to reduce cache misses
	//Use offsets in commands like vkCmdBindVertexBuffers
	VkBuffer m_VertexBuffer;
	VkDeviceMemory m_VertexBufferMemory;

	VkBuffer m_IndexBuffer;
	VkDeviceMemory m_IndexBufferMemory;

	uint16_t m_VertexCount;
	uint16_t m_IndexCount;
};