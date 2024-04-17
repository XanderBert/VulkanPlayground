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
#include "implot.h"
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


	std::shared_ptr<Material> material01 = MaterialManager::CreateMaterial(vulkanContext, "shader2D.vert", "shader2.frag", "MT_2D");
	std::shared_ptr<Material> material02 = MaterialManager::CreateMaterial(vulkanContext, "shader.vert", "shader.frag", "MT_Depth");


	m_Meshes.push_back(std::make_unique<Mesh>(vertices2, indices2, material01, "Simple Rectangle"));
	m_Meshes.back()->SetPosition(glm::vec3(-0.5f, 0.5f, 0.0f));
	m_Meshes.back()->SetScale(glm::vec3(0.6f));

	m_Meshes.push_back(std::make_unique<Mesh>("ball.obj", material01));
	m_Meshes.back()->SetPosition(glm::vec3(-0.8f, -0.8f, 0.0f));
	m_Meshes.back()->SetScale(glm::vec3(0.1f, 0.2f, 0.1f));


	m_Meshes.push_back(std::make_unique<Mesh>("viking.obj", material02));
	m_Meshes.push_back(std::make_unique<Mesh>("vehicle.obj", material02));
	m_Meshes.back()->SetPosition(glm::vec3{ -1.0f, 2.6f,0.f });
	m_Meshes.back()->SetRotation(glm::vec3{ 90.f, 0.f, 0.f });
	m_Meshes.back()->SetScale(glm::vec3(0.1f));



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
	ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);

	const ImGuiIO& io = ImGui::GetIO(); (void)io;
	const float ms = 1000.0f / io.Framerate;
	ImGui::Begin("Info");

	ImGui::Text("Application average %.3f ms", ms);
	ImGui::Text("DeltaTime: %.5f", GameTimer::GetDeltaTime());
	ImGui::Text("%.1f FPS", io.Framerate);

	static std::vector<float> frameTimes;
	if (ImPlot::BeginPlot("Frame Times", ImVec2(-1, 0), ImPlotFlags_NoInputs | ImPlotFlags_NoTitle))
	{
		//Update FrameTimes
		frameTimes.push_back(ms);
		if (frameTimes.size() > 1500) frameTimes.erase(frameTimes.begin());

		ImPlot::SetupAxes("time", "ms", ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_NoTickLabels, ImPlotAxisFlags_AutoFit);
		ImPlot::PlotLine("Frame Times", frameTimes.data(), frameTimes.size(), 0.001, 0, ImPlotLineFlags_Shaded);
		ImPlot::EndPlot();
	}


	ImGui::End();

	ShaderFactory::Render();
	VulkanLogger::Log.Render("Vulkan Log: ");
	MaterialManager::OnImGui();
	Camera::OnImGui();

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
