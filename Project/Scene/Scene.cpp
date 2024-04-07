#include "Scene.h"
#include "Mesh/Mesh.h"
#include "Mesh/Vertex.h"
#include "shaders/ShaderFactory.h"
#include "imgui.h"
#include <Core/Logger.h>
#include "../Mesh/ModelLoader.h"
#include <Timer/GameTimer.h>

#include "Input/Input.h"


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
	Input::BindFunction({ GLFW_KEY_A, Input::KeyType::Hold }, Camera::MoveRight);
	Input::BindFunction({ GLFW_KEY_D, Input::KeyType::Hold }, Camera::MoveLeft);

	//Input::BindFunction({ GLFW_KEY_Q, Input::KeyType::Hold }, Camera::MoveUp);
	//Input::BindFunction({ GLFW_KEY_E, Input::KeyType::Hold }, Camera::MoveDown);
	//Input::BindFunction({ GLFW_KEY_ESCAPE, Input::KeyType::Press }, []() { glfwSetWindowShouldClose(ServiceLocator::GetWindow()->GetWindow(), GLFW_TRUE); });


	Input::BindFunction({ GLFW_MOUSE_BUTTON_RIGHT, Input::KeyType::Press }, Camera::OnRightPressed);
	Input::AddMouseMovementListener(Camera::OnMouseMoved);
}


void Scene::Render(VkCommandBuffer commandBuffer) const
{
	GameTimer::UpdateDelta();
	//ShaderFactory::Render();


	const ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::Begin("Info");
	ImGui::Text("Application average %.3f ms", 1000.0f / io.Framerate);
	ImGui::Text("%.1f FPS", io.Framerate);
	ImGui::Text("DeltaTime: %.5f", GameTimer::GetDeltaTime());
	ImGui::DragFloat3("Camera Position", glm::value_ptr(Camera::GetPosition()), 0.1f);
	ImGui::End();


	VulkanLogger::Log.Render("Vulkan Log: ");

	ImGui::Render();

	for (const auto& mesh : m_Meshes)
	{
		mesh->Bind(commandBuffer);
		mesh->Render(commandBuffer);
	}
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
