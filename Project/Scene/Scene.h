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
	Scene();
	~Scene() = default;
	Scene(const Scene&) = delete;
	Scene& operator=(const Scene&) = delete;
	Scene(Scene&&) = delete;
	Scene& operator=(Scene&&) = delete;


	void Render(VkCommandBuffer commandBuffer) const;
	void AddMesh(const std::vector<Vertex>& vertices, const std::vector<uint16_t>& indices);

	void CleanUp() const;


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

	//void SetCamera(std::unique_ptr<Camera> pCamera);
	//Camera* GetCamera() const;


private:
	std::vector<std::unique_ptr<Mesh>> m_Meshes{};
	std::unique_ptr<Camera> m_pCamera;
};
