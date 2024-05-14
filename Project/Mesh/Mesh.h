#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include "Material.h"

class Material;
class VulkanContext;
struct Vertex;

class Mesh final
{
public:
	Mesh(const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices,
         const std::shared_ptr<Material> &material, const std::string &meshName);
    Mesh(const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices,
         const std::shared_ptr<Material> &material, std::string meshName);
    Mesh(const std::string& modelPath, const std::shared_ptr<Material> &material, const std::string& meshName = "");
	~Mesh() = default;

	Mesh(const Mesh&) = delete;
	Mesh(Mesh&&) = delete;
	Mesh& operator=(const Mesh&) = delete;
	Mesh& operator=(Mesh&&) = delete;

	void Bind(VkCommandBuffer commandBuffer);
	void OnImGui();
	void Render(VkCommandBuffer commandBuffer);
	void CleanUp();

	std::string GetMeshName() const { return m_MeshName; }
    Material* GetMaterial() const { return m_pMaterial.get(); }

	//TODO: Move to a component system
	void SetPosition(const glm::vec3& position);
	void SetScale(const glm::vec3& scale);
	void SetRotation(const glm::vec3& rotation);


private:
	void CreateVertexBuffer(const std::vector<Vertex>& vertices);
	void CreateIndexBuffer(const std::vector<uint32_t>& indices);

	VulkanContext* m_pContext;

	//TODO: Store the vertex and index buffer in 1 VkBuffer to reduce cache misses
	//Use offsets in commands like vkCmdBindVertexBuffers
	VkBuffer m_VertexBuffer;
	VkDeviceMemory m_VertexBufferMemory;

	VkBuffer m_IndexBuffer;
	VkDeviceMemory m_IndexBufferMemory;

	uint16_t m_VertexCount;
	uint16_t m_IndexCount;

	std::shared_ptr<Material> m_pMaterial;

	std::vector<uint16_t> m_VariableHandles;

	glm::mat4 m_ModelMatrix{1};
	std::string m_MeshName;


	bool m_Rotate = false;
	float m_RotationSpeed = 180.0f;

	bool m_Visible = true;
	bool m_VisibleBuffer = true;

    DescriptorSet m_MeshDescriptorSet{};
};