#include "Scene.h"
#include "Mesh/Mesh.h"
#include "Mesh/Vertex.h"
#include "shaders/ShaderFactory.h"
#include "imgui.h"
#include <Core/Logger.h>


Scene::Scene()
{
	//Temporary data
	//const std::vector<Vertex> vertices =
	//{
	//{{0.0f, -0.5f}, {1.0f, 1.0f, 1.0f}},
	//{{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
	//{{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
	//{ {0.0f, -0.5f}, {1.0f, 1.0f, 1.0f}},
	//{{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
	//{{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
	//};

	const std::vector<Vertex> vertices2 =
	{

	{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
	{{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
	{{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
	{{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
	};

	const std::vector<uint16_t> indices = {0, 1, 2, 2, 3, 0};

	//AddMesh(vertices);
	AddMesh(vertices2, indices);
}

void Scene::Render(VkCommandBuffer commandBuffer) const
{
	//ShaderFactory::Render();

	//Create window and render FPS
	ImGui::Begin("Another Window");   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
	ImGui::End();

	VulkanLogger::Log.Render("Vulkan Log: ");

	ImGui::Render();

	for (const auto& mesh : m_Meshes)
	{
		mesh->Bind(commandBuffer);
		mesh->Render(commandBuffer);
	}
}

void Scene::AddMesh(const std::vector<Vertex>& vertices, const std::vector<uint16_t>& indices)
{
	m_Meshes.push_back(std::make_unique<Mesh>(vertices, indices));
}

void Scene::CleanUp() const
{
	for (const auto& mesh : m_Meshes)
	{
		mesh->CleanUp();
	}
}
