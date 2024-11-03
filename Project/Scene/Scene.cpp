#include "Scene.h"

#include <implot.h>

#include "Core/CommandBuffer.h"
#include "Core/DepthResource.h"
#include "Core/GBuffer.h"
#include "Core/GlobalDescriptor.h"
#include "Core/ImGuiWrapper.h"
#include "Core/Logger.h"
#include "Core/SwapChain.h"
#include "Input/Input.h"
#include "Mesh/MaterialManager.h"
#include "Mesh/Mesh.h"
#include "Mesh/ModelLoader.h"
#include "Mesh/Vertex.h"
#include "Patterns/ServiceLocator.h"
#include "Timer/GameTimer.h"
#include "glm/ext/matrix_transform.hpp"
#include "shaders/Logic/ShaderEditor.h"
#include "shaders/Logic/ShaderFactory.h"


Scene::Scene(VulkanContext* vulkanContext)
{
	auto [width, height] = SwapChain::Extends();

    //
    //Create Materials
    //
    std::shared_ptr<Material> depthMaterial = MaterialManager::CreateMaterial(vulkanContext, "DepthOnlyMaterial");
    depthMaterial->SetDepthOnly(true);
	depthMaterial->AddShader("depth.vert", ShaderType::VertexShader);
	depthMaterial->AddShader("depth.frag", ShaderType::FragmentShader);


    std::shared_ptr<Material> PBR_Material = MaterialManager::CreateMaterial(vulkanContext, "shader.vert", "shader.frag", "PBR_Material");
    auto* ubo = PBR_Material->GetDescriptorSet()->AddBuffer(0, DescriptorType::UniformBuffer);
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


	std::shared_ptr<Material> DownSampleDeptBufferMaterial = MaterialManager::CreateMaterial(vulkanContext, "DownSample");
	DownSampleDeptBufferMaterial->AddShader("DownSample.comp", ShaderType::ComputeShader);
	DownSampleDeptBufferMaterial->GetDescriptorSet()->AddGBuffer(0, 1);
	std::shared_ptr<Texture> downSampleTexture = DownSampleDeptBufferMaterial->GetDescriptorSet()->CreateOutputTexture(2, vulkanContext, {width / 2.0f,  height / 2.0f});



	glm::vec2 pixelSize{2.0f / static_cast<float>(width), 2.0f / static_cast<float>(height)};

	std::shared_ptr<Material> computeMaterial = MaterialManager::CreateMaterial(vulkanContext, "ComputeMaterial");
	computeMaterial->AddShader("test.comp", ShaderType::ComputeShader);

	auto* computeUbo = computeMaterial->GetDescriptorSet()->AddBuffer(0, DescriptorType::UniformBuffer);
	auto nearPlaneSizeHandle = computeUbo->AddVariable({Camera::GetNearPlaneSizeAtDistance(1.0f), 0,0});
	auto aspectRatioHandle = computeUbo->AddVariable({Camera::GetAspectRatio(), 0,0, 0});
	auto radiusWorldHandle = computeUbo->AddVariable({1, 0,0,0});
	auto maxRadiusScreenHandle = computeUbo->AddVariable({0.1,0,0,0});
	auto pixelSizeHandle = computeUbo->AddVariable({pixelSize.x,pixelSize.y,0,0});

	computeMaterial->GetDescriptorSet()->AddGBuffer(1,2);
	std::shared_ptr<Texture> SSAO = computeMaterial->GetDescriptorSet()->CreateOutputTexture(3, vulkanContext, {width / 2.0f, height / 2.0f});
	computeMaterial->GetDescriptorSet()->AddTexture(4, downSampleTexture, vulkanContext);

	SwapChain::OnSwapChainRecreated.AddLambda([aspectRatioHandle, pixelSizeHandle, nearPlaneSizeHandle](const VulkanContext* context)
	{
		const auto extends = SwapChain::Extends();
		const auto aspect = static_cast<float>(extends.width) / static_cast<float>(extends.height);
		const glm::vec2 pixelSize{2.0f / static_cast<float>(extends.width), 2.0f / static_cast<float>(extends.height)};

		auto* computeUbo = MaterialManager::GetMaterial("ComputeMaterial")->GetDescriptorSet()->GetBuffer(0);
		computeUbo->UpdateVariable(aspectRatioHandle, glm::vec4(aspect, 0,0,0));
		computeUbo->UpdateVariable(pixelSizeHandle, glm::vec4{pixelSize.x, pixelSize.y, 0,0});
		computeUbo->UpdateVariable(nearPlaneSizeHandle, glm::vec4{Camera::GetNearPlaneSizeAtDistance(1.0f), 0,0});
	});





	std::shared_ptr<Material> UpSampleMaterial = MaterialManager::CreateMaterial(vulkanContext, "UpSample");
	UpSampleMaterial->AddShader("UpSample.comp", ShaderType::ComputeShader);
	UpSampleMaterial->GetDescriptorSet()->AddGBuffer(0, 1);
	UpSampleMaterial->GetDescriptorSet()->AddTexture(2, SSAO, vulkanContext);
	std::shared_ptr<Texture> upSampleTexture = UpSampleMaterial->GetDescriptorSet()->CreateOutputTexture(3, vulkanContext, {width, height});


	//Composite Material
	std::shared_ptr<Material> compositeMaterial = MaterialManager::CreateMaterial(vulkanContext, "composite.vert", "composite.frag", "CompositeMaterial");
	compositeMaterial->SetIsComposite(true);
	compositeMaterial->GetDescriptorSet()->AddColorAttachment(GBuffer::GetAlbedoAttachment(), 0);
	compositeMaterial->GetDescriptorSet()->AddTexture(1, upSampleTexture, vulkanContext);
	auto compositeBuffer = compositeMaterial->GetDescriptorSet()->AddBuffer(2, DescriptorType::UniformBuffer);
	auto sizeHandle = compositeBuffer->AddVariable(glm::vec4{width, height, 0,0});

	SwapChain::OnSwapChainRecreated.AddLambda([sizeHandle](const VulkanContext* context)
	{
		const auto extends = SwapChain::Extends();
		auto* compositeBuffer = MaterialManager::GetMaterial("CompositeMaterial")->GetDescriptorSet()->GetBuffer(2);
		compositeBuffer->UpdateVariable(sizeHandle, glm::vec4{extends.width, extends.height, 0,0});

		LogWarning(std::to_string(extends.width) + " " + std::to_string(extends.height));
	});



	//Load model
	GLTFLoader::LoadGLTF("FlightHelmet.gltf", this, vulkanContext);

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



void Scene::AlbedoRender(VkCommandBuffer commandBuffer) const
{
	auto mat = MaterialManager::GetMaterial("CompositeMaterial");

	GlobalDescriptor::Bind(ServiceLocator::GetService<VulkanContext>(), commandBuffer, mat->GetPipelineLayout(), PipelineType::Compute);
	mat->Bind(commandBuffer, Camera::GetViewMatrix());
	vkCmdDraw(commandBuffer, 3, 1, 0, 0);
}

void Scene::Render(VkCommandBuffer commandBuffer) const
{
	GameTimer::UpdateDelta();
	Camera::Update();



    ShaderEditor::Render();

	const ImGuiIO& io = ImGui::GetIO(); (void)io;
	const float ms = 1000.0f / io.Framerate;
	ImGui::Begin("Info");


	ImGui::Text("%.1f FPS", io.Framerate);
    // ImGui::SameLine();
    // if(ImGui::Button("Generate Memory Layout"))
    // {
	//		auto* pContext = ServiceLocator::GetService<VulkanContext>();
    //     Allocator::GenerateMemoryLayout(pContext);
    // }
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
	Camera::OnImGui();
	GBuffer::OnImGui();

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

void Scene::ExecuteComputePass(VkCommandBuffer commandBuffer) const
{
	auto extends = SwapChain::Extends();
	constexpr uint32_t groupSize = 32;
	uint32_t fullscreenGroupCountX = (extends.width + groupSize - 1) / groupSize;
	uint32_t fullscreenGroupCountY = (extends.height + groupSize - 1) / groupSize;

	uint32_t quarterGroupCountX = (extends.width / 4 + groupSize - 1) / groupSize;
	uint32_t quarterGroupCountY = (extends.height / 4 + groupSize - 1) / groupSize;

	Texture* downSampleTexture{};
	Texture* SSAO{};



	auto downSampleMaterial = MaterialManager::GetMaterial("DownSample");
	if(downSampleMaterial->IsCompute())
	{
		const auto& textures = downSampleMaterial->GetDescriptorSet()->GetTextures();
		//Transition the output texture to writable
		for(const auto& texture : textures)
		{
			if(texture->IsOutputTexture())
				downSampleTexture = texture.get();
		}

		downSampleTexture->TransitionToGeneralImageLayout(commandBuffer);

		GlobalDescriptor::Bind(ServiceLocator::GetService<VulkanContext>(), commandBuffer, downSampleMaterial->GetPipelineLayout(), PipelineType::Compute);
		downSampleMaterial->Bind(commandBuffer, Camera::GetProjectionMatrix());
		vkCmdDispatch(commandBuffer, fullscreenGroupCountX, fullscreenGroupCountY, 1);

		downSampleTexture->TransitionToReadableImageLayout(commandBuffer);
		downSampleTexture->SetOutputTexture(false);
	}

	//Execute the SSAO Shader
	auto ssaoMaterial = MaterialManager::GetMaterial("ComputeMaterial");
    if(ssaoMaterial->IsCompute())
    {
        const auto& textures = ssaoMaterial->GetDescriptorSet()->GetTextures();
       	//Transition the output texture to writable
        for(const auto& texture : textures)
		{
			if(texture->IsOutputTexture()) //But what if we pass a output texture as a input texture?
				SSAO = texture.get();
			SSAO->TransitionToGeneralImageLayout(commandBuffer);
        }

		GlobalDescriptor::Bind(ServiceLocator::GetService<VulkanContext>(), commandBuffer, ssaoMaterial->GetPipelineLayout(), PipelineType::Compute);
		ssaoMaterial->Bind(commandBuffer, Camera::GetViewMatrix());
		vkCmdDispatch(commandBuffer, quarterGroupCountX, quarterGroupCountY	, 1);

    	SSAO->TransitionToReadableImageLayout(commandBuffer);
		SSAO->SetOutputTexture(false);
    }



	auto upSampleMaterial = MaterialManager::GetMaterial("UpSample");
	if(upSampleMaterial->IsCompute())
	{
		const auto& textures = upSampleMaterial->GetDescriptorSet()->GetTextures();
		//Transition the output texture to writable
		for(const auto& texture : textures)
		{
			if(texture->IsOutputTexture())
				texture->TransitionToGeneralImageLayout(commandBuffer);
		}

		GlobalDescriptor::Bind(ServiceLocator::GetService<VulkanContext>(), commandBuffer, upSampleMaterial->GetPipelineLayout(), PipelineType::Compute);
		upSampleMaterial->Bind(commandBuffer, Camera::GetProjectionMatrix());
		vkCmdDispatch(commandBuffer, fullscreenGroupCountX, fullscreenGroupCountY, 1);

		//transition the output texture to readable
		for(const auto& texture : textures)
		{
			if(texture->IsOutputTexture())
				texture->TransitionToReadableImageLayout(commandBuffer);
		}
	}

	downSampleTexture->SetOutputTexture(true);
	SSAO->SetOutputTexture(true);
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

std::vector<Mesh *> Scene::GetMeshes() const {
    std::vector<Mesh*> meshes{};
    meshes.reserve(m_Meshes.size());
    for (const auto& mesh : m_Meshes)
    {
        meshes.push_back(mesh.get());
    }
    return meshes;
}
