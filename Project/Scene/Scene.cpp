#include "Scene.h"

#include <implot.h>

#include "Mesh/Mesh.h"
#include "Mesh/ModelLoader.h"
#include "Mesh/Vertex.h"
#include "shaders/Logic/ShaderFactory.h"
#include "Core/ImGuiWrapper.h"
#include "Core/Logger.h"
#include "Mesh/MaterialManager.h"
#include "Timer/GameTimer.h"
#include "Input/Input.h"
#include "Core/GlobalDescriptor.h"
#include "Patterns/ServiceLocator.h"
#include "shaders/Logic/ShaderEditor.h"


Scene::Scene(VulkanContext* vulkanContext)
{
    //
    //Create Materials
    //
    std::shared_ptr<Material> depthMaterial = MaterialManager::CreateMaterial(vulkanContext, "depth.vert", "depth.frag", "DepthOnlyMaterial");
    depthMaterial->SetDepthOnly(true);


    std::shared_ptr<Material> PBR_Material = MaterialManager::CreateMaterial(vulkanContext, "shader.vert", "shader.frag", "PBR_Material");
    auto* ubo = PBR_Material->GetDescriptorSet()->AddUniformBuffer(0);
    ubo->AddVariable(glm::vec4{1});
    ubo->AddVariable(glm::vec4{1});
    PBR_Material->GetDescriptorSet()->AddTexture(1, "Robot_Albedo.jpg", vulkanContext, ColorType::SRGB);
    PBR_Material->GetDescriptorSet()->AddTexture(2, "Robot_Normal.jpg", vulkanContext, ColorType::LINEAR);
    PBR_Material->GetDescriptorSet()->AddTexture(3, "Robot_Roughness.jpg", vulkanContext, ColorType::LINEAR);
    PBR_Material->GetDescriptorSet()->AddTexture(4, "Robot_Metal.jpg", vulkanContext, ColorType::LINEAR);
    PBR_Material->GetDescriptorSet()->AddTexture(5, "cubemap_vulkan.ktx", vulkanContext, ColorType::SRGB, TextureType::TEXTURE_CUBE);

    std::shared_ptr<Material> skyboxMaterial = MaterialManager::CreateMaterial(vulkanContext, "skybox.vert", "skybox.frag", "Skybox_Material");
    skyboxMaterial->GetDescriptorSet()->AddTexture(1, "cubemap_vulkan.ktx", vulkanContext, ColorType::SRGB, TextureType::TEXTURE_CUBE);
    skyboxMaterial->SetCullMode(VK_CULL_MODE_FRONT_BIT);





   //GLTFLoader::LoadGLTF("FlightHelmet.gltf", this, vulkanContext);
   //GLTFLoader::LoadGLTF("Assets/CesiumMan.gltf", this, vulkanContext);
   //GLTFLoader::LoadGLTF("Assets/sponza.gltf", this, vulkanContext);




    //
    // Create Meshes
    //
	// m_Meshes.push_back(std::make_unique<Mesh>("Robot.obj", PBR_Material, "Robot"));
	// m_Meshes.back()->SetPosition(glm::vec3{ -1.0f, 2.6f,0.f });
	// m_Meshes.back()->SetRotation(glm::vec3{ 90.f, 0.f, 0.f });
	// m_Meshes.back()->SetScale(glm::vec3(0.1f));



    //Create the cubmap last so its rendered last
    //with a simple shader & depthmap trick we can make it only over the fragments that are not yet written to
    m_Meshes.push_back(std::make_unique<Mesh>("Cube.obj", skyboxMaterial, depthMaterial, "CubeMap"));
    m_Meshes.back()->SetRotation(glm::vec3{-90,0,0});

    //
    // Setup Input
    //
	Input::BindFunction({ {GLFW_KEY_W}, Input::InputType::Hold }, Camera::MoveForward);
	Input::BindFunction({ {GLFW_KEY_S}, Input::InputType::Hold }, Camera::MoveBackward);
	Input::BindFunction({ {GLFW_KEY_A}, Input::InputType::Hold }, Camera::MoveRight);
	Input::BindFunction({ {GLFW_KEY_D}, Input::InputType::Hold }, Camera::MoveLeft);
	Input::BindFunction({ {GLFW_MOUSE_BUTTON_RIGHT}, Input::InputType::Press }, Camera::OnRightPressed);
	Input::AddMouseMovementListener(Camera::OnMouseMoved);


    Input::BindFunction({{GLFW_KEY_T}, Input::InputType::Press}, ImGuizmoHandler::RotateOperation);
    Input::BindFunction({{GLFW_KEY_Y}, Input::InputType::Press}, ImGuizmoHandler::ScaleOperation);

    Input::BindFunction({{GLFW_KEY_S}, Input::InputType::Press, GLFW_MOD_CONTROL}, ShaderEditor::SaveFile);
    //Input::BindFunction({{GLFW_KEY_I}, Input::InputType::Press, GLFW_MOD_CONTROL}, ImGuiWrapper::OpenFileDialog);

    LogInfo("Scene Made");
}


void Scene::RenderDepth(VkCommandBuffer commandBuffer) const
{
    for (const auto& mesh : m_Meshes)
    {
        mesh->BindDepth(commandBuffer);
        mesh->Render(commandBuffer);
    }
}

void Scene::Render(VkCommandBuffer commandBuffer) const
{
	GameTimer::UpdateDelta();

    //TODO: This check should only happen on events / not in the hot code path
    ShaderManager::ReloadNeededShaders(ServiceLocator::GetService<VulkanContext>());
    ShaderEditor::Render();

	const ImGuiIO& io = ImGui::GetIO(); (void)io;
	const float ms = 1000.0f / io.Framerate;
	ImGui::Begin("Info");


	ImGui::Text("%.1f FPS", io.Framerate);
    ImGui::SameLine();
    if(ImGui::Button("Generate Memory Layout"))
    {
        Allocator::GenerateMemoryLayout(ServiceLocator::GetService<VulkanContext>());
    }
    // if(Allocator::MemoryLayoutTexture.has_value())
    // {
    //     Allocator::MemoryLayoutTexture.value()->OnImGui();
    // }

	static std::vector<float> frameTimes;
	if (ImPlot::BeginPlot("Frame Times", ImVec2(-1, 0), ImPlotFlags_NoInputs | ImPlotFlags_NoTitle))
	{
		//Update FrameTimes
		frameTimes.push_back(ms);
		if (frameTimes.size() > 1500) frameTimes.erase(frameTimes.begin());

		ImPlot::SetupAxes("time", "ms", ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_NoTickLabels, ImPlotAxisFlags_AutoFit);
		ImPlot::PlotLine("Frame Times", frameTimes.data(), static_cast<int>(frameTimes.size()), 0.001, 0, ImPlotLineFlags_Shaded);
		ImPlot::EndPlot();
	}
    ImGui::Text("Application average %.3f ms", ms);

	ImGui::End();

	ShaderFactory::Render();
	VulkanLogger::Log.Render("Vulkan Log: ");
	MaterialManager::OnImGui();

    GlobalDescriptor::OnImGui();


    Camera::Update();
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
    for (const auto &mesh: m_Meshes) {
        mesh->CleanUp();
    }
}
void Scene::AddMesh(std::unique_ptr<Mesh> mesh)
{
    m_Meshes.push_back(std::move(mesh));
}
