#pragma once
#include <vector>
#include <memory>
#include "Camera/Camera.h"
#include <vulkan/vulkan.h>
#include "Mesh/Mesh.h"


struct Vertex;


class Scene final
{
public:
	Scene(VulkanContext* vulkanContext);
	~Scene() = default;
	Scene(const Scene&) = delete;
	Scene& operator=(const Scene&) = delete;
	Scene(Scene&&) = delete;
	Scene& operator=(Scene&&) = delete;

    void RenderDepth(VkCommandBuffer commandBuffer) const;
	void Render(VkCommandBuffer commandBuffer) const;


	void CleanUp() const;

    void AddMesh(std::unique_ptr<Mesh> mesh);


	std::vector<Mesh*> GetMeshes() const
	{
		std::vector<Mesh*> meshes{};
		meshes.reserve(m_Meshes.size());
		for (const auto& mesh : m_Meshes)
		{
			meshes.push_back(mesh.get());
		}
		return meshes;
	}

private:
	std::vector<std::unique_ptr<Mesh>> m_Meshes{};


};
