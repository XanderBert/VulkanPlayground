#include "Scene.h"

#include "Mesh/Mesh.h"
#include "Mesh/ModelLoader.h"
#include "Mesh/Vertex.h"

#include "shaders/Logic/ShaderFactory.h"
#include "Core/ImGuiWrapper.h"
#include "Core/Logger.h"

#include <Timer/GameTimer.h>
#include "Input/Input.h"
#include <Mesh/MaterialManager.h>

#include "ImGuizmo.h"
#include "Patterns/ServiceLocator.h"


Scene::Scene(VulkanContext* vulkanContext)
{
	const std::vector<Vertex> vertices2 = 
{
	{{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
	{{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
	{{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
	{{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},

	{{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
	{{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
	{{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
	{{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}
	};

	const std::vector<uint32_t> indices2 = {
		0, 1, 2, 2, 3, 0,
		4, 5, 6, 6, 7, 4
	};


	std::shared_ptr<Material> material01 = MaterialManager::CreateMaterial(vulkanContext, "shader2D.vert", "shader.frag", "MT_2D");
	std::shared_ptr<Material> material02 = MaterialManager::CreateMaterial(vulkanContext, "shader.vert", "shader.frag", "MT_Depth");

	//std::shared_ptr<Material> material02 = MaterialManager::CreateMaterial(vulkanContext);
	//material02->AddShader("shader.vert", ShaderType::VertexShader);
	//material02->AddShader("shader2.frag", ShaderType::FragmentShader);

	m_Meshes.push_back(std::make_unique<Mesh>(vertices2, indices2, material01, "Simple Rectangle"));
	m_Meshes.push_back(std::make_unique<Mesh>("viking.obj", material02));


	Input::BindFunction({ GLFW_KEY_W, Input::KeyType::Hold }, Camera::MoveForward);
	Input::BindFunction({ GLFW_KEY_S, Input::KeyType::Hold }, Camera::MoveBackward);
	Input::BindFunction({ GLFW_KEY_A, Input::KeyType::Hold }, Camera::MoveRight);
	Input::BindFunction({ GLFW_KEY_D, Input::KeyType::Hold }, Camera::MoveLeft);
	Input::BindFunction({ GLFW_MOUSE_BUTTON_RIGHT, Input::KeyType::Press }, Camera::OnRightPressed);
	Input::AddMouseMovementListener(Camera::OnMouseMoved);
}


void Scene::Render(VkCommandBuffer commandBuffer) const
{
	GameTimer::UpdateDelta();

	
	static bool showShaderFactory = false;
	static bool showLogger = true;

	const ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::Begin("Info");
	ImGui::Text("Application average %.3f ms", 1000.0f / io.Framerate);
	ImGui::Text("DeltaTime: %.5f", GameTimer::GetDeltaTime());
	ImGui::Text("%.1f FPS", io.Framerate);
	ImGui::DragFloat3("Camera Position", glm::value_ptr(Camera::GetPosition()), 0.1f);

	ImGui::Checkbox("Show Shader Factory", &showShaderFactory);
	ImGui::Checkbox("Show Log", &showLogger);
	ImGui::End();

	if(showShaderFactory) ShaderFactory::Render();
	if(showLogger) 	VulkanLogger::Log.Render("Vulkan Log: ");


	MaterialManager::OnImGui();

	for (const auto& mesh : m_Meshes)
	{
		mesh->Bind(commandBuffer);

		ImGui::Begin("Mesh");
		if(ImGui::CollapsingHeader(mesh->GetMeshName().c_str()))
		{
			mesh->OnImGui();
		}
		ImGui::End();

		mesh->Render(commandBuffer);
	}

	ImGui::Render();
}

void Scene::CleanUp() const
{
	for (const auto& mesh : m_Meshes)
	{
		mesh->CleanUp();
	}
}
