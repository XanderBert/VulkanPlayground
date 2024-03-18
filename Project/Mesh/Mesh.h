#pragma once
#include "vulkan/vulkan.h"
#include <vector>

class VulkanContext;
struct Vertex;
class Mesh final
{
public:
	Mesh(const std::vector<Vertex>& vertices);
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

	VulkanContext* m_pContext;

	VkBuffer m_VertexBuffer;
	VkDeviceMemory m_VertexBufferMemory;

	uint32_t m_VertexCount;
};