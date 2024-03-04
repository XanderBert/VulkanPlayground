#pragma once
#include <vector>

#include "Vertex.h"

class Mesh final
{
public:
	Mesh(const VkDevice& pDevice, const std::vector<Vertex>& vertices, VkPhysicalDevice physicalDevice);
	~Mesh();

	Mesh(const Mesh&) = delete;
	Mesh(Mesh&&) = delete;
	Mesh& operator=(const Mesh&) = delete;
	Mesh& operator=(Mesh&&) = delete;

	void Bind(VkCommandBuffer commandBuffer) const;
	void Render(VkCommandBuffer commandBuffer) const;

private:
	void CreateVertexBuffer(const std::vector<Vertex>& vertices, VkPhysicalDevice physicalDevice);

	VkDevice m_pDevice;

	VkBuffer m_VertexBuffer;
	VkDeviceMemory m_VertexBufferMemory;
	uint32_t m_VertexCount;
};