#pragma once
#include <memory>

#include "Mesh/Mesh.h"

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
	void AddMesh(const std::vector<Vertex>& vertices);

	void CleanUp() const;

private:
	std::vector<std::unique_ptr<Mesh>> m_Meshes{};
};
