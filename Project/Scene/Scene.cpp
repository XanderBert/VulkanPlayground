#include "Scene.h"

#include "imgui.h"
#include "../Mesh/Vertex.h"
#include "../Mesh/Mesh.h"
#include "shaders/ShaderFactory.h"

Scene::Scene()
{
	//Temporary data
	const std::vector<Vertex> vertices =
	{
	{{0.0f, -0.5f}, {1.0f, 1.0f, 1.0f}},
	{{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
	{{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
	{ {0.0f, -0.5f}, {1.0f, 1.0f, 1.0f}},
	{{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
	{{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
	};

	const std::vector<Vertex> vertices2 =
	{
	{{1.0f, -0.5f}, {1.0f, 0.0f, 1.0f}},
	{{1.5f, 0.5f}, {1.0f, 1.0f, 0.0f}},
	{{-1.5f, 0.5f}, {0.0f, 1.0f, 1.0f}},
	{ {1.0f, -0.5f}, {1.0f, 0.0f, 1.0f}},
	{{1.5f, 0.5f}, {1.0f, 1.0f, 0.0f}},
	{{-1.5f, 0.5f}, {0.0f, 1.0f, 1.0f}}
	};

	AddMesh(vertices);
	AddMesh(vertices2);
}

void Scene::Render(VkCommandBuffer commandBuffer) const
{
	ShaderFactory::Render();
	ImGui::Render();

	for (const auto& mesh : m_Meshes)
	{
		mesh->Bind(commandBuffer);
		mesh->Render(commandBuffer);
	}
}

void Scene::AddMesh(const std::vector<Vertex>& vertices)
{
	m_Meshes.push_back(std::make_unique<Mesh>(vertices));
}

void Scene::CleanUp() const
{
	for (const auto& mesh : m_Meshes)
	{
		mesh->CleanUp();
	}
}
