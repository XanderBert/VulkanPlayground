#pragma once
#include <memory>
#include <vector>
#include <vulkan/vulkan.h>
#include "Material.h"

class Material;
class VulkanContext;
struct Vertex;

struct Primitive
{
	uint32_t firstIndex;
	uint32_t indexCount;
	std::shared_ptr<Material> material;

	inline void Render(VkCommandBuffer commandBuffer, const glm::mat4& modelMatrix) const
	{
		//TODO: Move pushconstant to a UBO
		//material->BindPushConstant(commandBuffer, modelMatrix);
		material->Bind(commandBuffer);
		vkCmdDrawIndexed(commandBuffer, indexCount, 1, firstIndex, 0, 0);
	}
};

class Mesh final
{
public:
    Mesh(const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices, std::string meshName, const std::vector<Primitive>& primitives, uint32_t firstDrawIndex = 0, int32_t vertexOffset = 0);
    Mesh(const std::string& modelPath,const std::string& materialName, const std::string& meshName = "");


    ~Mesh() = default;

	Mesh(const Mesh&) = delete;
	explicit Mesh(Mesh&& other) noexcept = delete;
	Mesh& operator=(const Mesh&) = delete;
	Mesh& operator=(Mesh&&) = delete;

	void Render(VkCommandBuffer commandBuffer);
	void RenderDepth(VkCommandBuffer commandBuffer);

	void OnImGui();
	void CleanUp();

	[[nodiscard]] std::string GetMeshName() const { return m_MeshName; }
    [[nodiscard]] Material* GetMaterial() const { return m_pMaterial.get(); }

	//TODO: Move to a component system
	void SetPosition(const glm::vec3& position);
	void SetScale(const glm::vec3& scale);
	void SetRotation(const glm::vec3& rotation);

    glm::mat4 GetTransform() const { return m_ModelMatrix; }
    void SetTransform(const glm::mat4& transform) { m_ModelMatrix = transform; }

private:
	void CreateVertexBuffer(const std::vector<Vertex>& vertices);
	void CreateIndexBuffer(const std::vector<uint32_t>& indices);

	uint32_t m_IndexCount{};
    uint32_t m_FirstDrawIndex{};
    int32_t m_VertexOffset{};

	VulkanContext* m_pContext;

	//TODO: Store the vertex and index buffer in 1 VkBuffer to reduce cache misses
	std::vector<Primitive> m_Primitives{};
    Buffer m_VertexBuffer{};
	Buffer m_IndexBuffer{};

	std::shared_ptr<Material> m_pMaterial;
    std::shared_ptr<Material> m_pDepthMaterial;

	std::vector<uint16_t> m_VariableHandles;
	glm::mat4 m_ModelMatrix{1};
	std::string m_MeshName;


	bool m_Rotate = false;
	float m_RotationSpeed = 180.0f;

	bool m_Visible = true;
	bool m_VisibleBuffer = true;

    DescriptorSet m_MeshDescriptorSet{};
};