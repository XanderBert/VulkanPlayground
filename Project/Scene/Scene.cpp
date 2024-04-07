#include "Scene.h"

#include <chrono>

#include "Mesh/Mesh.h"
#include "Mesh/Vertex.h"
#include "shaders/ShaderFactory.h"
#include "imgui.h"
#include <Core/Logger.h>
#include "../Mesh/ModelLoader.h"


Scene::Scene()
{
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	ObjLoader::LoadObj("viking.obj", vertices, indices);

	//const std::vector<Vertex> vertices = {
	//{{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
	//{{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
	//{{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
	//{{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},

	//{{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
	//{{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
	//{{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
	//{{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}
	//};

	//const std::vector<uint32_t> indices = {
	//	0, 1, 2, 2, 3, 0,
	//	4, 5, 6, 6, 7, 4
	//};



	AddMesh(vertices, indices);

	Input::BindFunction({ GLFW_KEY_W, Input::KeyType::Hold }, Camera::MoveForward);
	Input::BindFunction({ GLFW_KEY_S, Input::KeyType::Hold }, Camera::MoveBackward);
	Input::BindFunction({ GLFW_KEY_A, Input::KeyType::Hold }, Camera::MoveLeft);
	Input::BindFunction({ GLFW_KEY_D, Input::KeyType::Hold }, Camera::MoveRight);
}

void Scene::Render(VkCommandBuffer commandBuffer) const
{
	static auto startTime = std::chrono::high_resolution_clock::now();
	const auto currentTime = std::chrono::high_resolution_clock::now();
	const float deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
	Camera::Update(deltaTime);


	//ShaderFactory::Render();

	//Create window and render FPS
	ImGui::Begin("Info");   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
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

	startTime = currentTime;
}

void Scene::AddMesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
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
