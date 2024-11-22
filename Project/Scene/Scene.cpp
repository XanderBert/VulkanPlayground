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
#include "shaders/Logic/ShaderEditor.h"
#include "shaders/Logic/ShaderFactory.h"


Scene::Scene(VulkanContext* vulkanContext)
{
	auto [width, height] = SwapChain::Extends();


    //
    //Depth Pass
    //
    std::shared_ptr<Material> depthMaterial = MaterialManager::CreateMaterial(vulkanContext, "DepthOnlyMaterial");
    depthMaterial->SetDepthOnly(true);
	depthMaterial->AddShader("depth.vert", ShaderType::VertexShader);
	depthMaterial->AddShader("depth.frag", ShaderType::FragmentShader);

	//
	//Skybox Material
	//
    std::shared_ptr<Material> skyboxMaterial = MaterialManager::CreateMaterial(vulkanContext, "skybox.vert", "skybox.frag", "Skybox_Material");
    skyboxMaterial->GetDescriptorSet()->AddTexture(1, "cubemap_vulkan.ktx", vulkanContext, ColorType::SRGB, TextureType::TEXTURE_CUBE);
    skyboxMaterial->SetCullMode(VK_CULL_MODE_FRONT_BIT);

	//
	//SSAO Materials
	//
	glm::ivec2 quarterScreen = glm::ivec2(width / 2.0f, height / 2.0f);


	//Downsample Depth
	//
	std::shared_ptr<Material> DownSampleDeptBufferMaterial = MaterialManager::CreateMaterial(vulkanContext, "DownSample");
	DownSampleDeptBufferMaterial->AddShader("DownSample.comp", ShaderType::ComputeShader);
	DownSampleDeptBufferMaterial->GetDescriptorSet()->AddDepthBuffer(0);
	std::shared_ptr<Texture> downSampleTexture = DownSampleDeptBufferMaterial->GetDescriptorSet()->CreateOutputTexture(1, vulkanContext, quarterScreen, ColorType::R16U);

	DynamicBuffer* downUbo = DownSampleDeptBufferMaterial->GetDescriptorSet()->AddBuffer(2, DescriptorType::UniformBuffer);
	downUbo->AddVariable({quarterScreen.x, quarterScreen.y,0,0});

	SwapChain::OnSwapChainRecreated.AddLambda([](const VulkanContext* context)
	{
		const auto extends = SwapChain::Extends();
		DynamicBuffer* downUbo = MaterialManager::GetMaterial("DownSample")->GetDescriptorSet()->GetBuffer(2);
		downUbo->UpdateVariable(0, {extends.width / 2.0f, extends.height / 2.0f,0,0});
	});






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
	std::shared_ptr<Texture> SSAO = computeMaterial->GetDescriptorSet()->CreateOutputTexture(3, vulkanContext, {width / 2.0f,  height / 2.0f});
	computeMaterial->GetDescriptorSet()->AddTexture(4, downSampleTexture, vulkanContext);

	SwapChain::OnSwapChainRecreated.AddLambda([aspectRatioHandle, pixelSizeHandle, nearPlaneSizeHandle](const VulkanContext* context)
	{
		const auto extends = SwapChain::Extends();
		const auto aspect = static_cast<float>(extends.width) / static_cast<float>(extends.height);
		const glm::vec2 pixelSize{static_cast<float>(extends.width), static_cast<float>(extends.height)};

		auto* computeUbo = MaterialManager::GetMaterial("ComputeMaterial")->GetDescriptorSet()->GetBuffer(0);
		computeUbo->UpdateVariable(aspectRatioHandle, glm::vec4(aspect, 0,0,0));
		computeUbo->UpdateVariable(pixelSizeHandle, glm::vec4{pixelSize.x, pixelSize.y, 0,0});
		computeUbo->UpdateVariable(nearPlaneSizeHandle, glm::vec4{Camera::GetNearPlaneSizeAtDistance(1.0f), 0,0});
	});


	std::shared_ptr<Material> BlurMaterial = MaterialManager::CreateMaterial(vulkanContext, "Blur");
	BlurMaterial->AddShader("Blur.comp", ShaderType::ComputeShader);
	BlurMaterial->GetDescriptorSet()->AddTexture(0, SSAO, vulkanContext);
	BlurMaterial->GetDescriptorSet()->AddTexture(1, downSampleTexture, vulkanContext);
	std::shared_ptr<Texture> blurredSSAO = BlurMaterial->GetDescriptorSet()->CreateOutputTexture(2, vulkanContext, {width / 2.0f, height / 2.0f});


	//Upsample
	std::shared_ptr<Material> UpSampleMaterial = MaterialManager::CreateMaterial(vulkanContext, "UpSample");
	UpSampleMaterial->AddShader("UpSample.comp", ShaderType::ComputeShader);
	UpSampleMaterial->GetDescriptorSet()->AddGBuffer(0, 1);
	UpSampleMaterial->GetDescriptorSet()->AddTexture(2, blurredSSAO, vulkanContext);
	std::shared_ptr<Texture> upSampleTexture = UpSampleMaterial->GetDescriptorSet()->CreateOutputTexture(3, vulkanContext, {width, height});






	//Composite Material
	std::shared_ptr<Material> compositeMaterial = MaterialManager::CreateMaterial(vulkanContext, "composite.vert", "composite.frag", "CompositeMaterial");
	compositeMaterial->SetIsComposite(true);
	compositeMaterial->GetDescriptorSet()->AddColorAttachment(GBuffer::GetAlbedoAttachment(), 0);

	//TODO blurred SSAO gets pushed in here
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
    m_Meshes.push_back(std::make_unique<Mesh>("Cube.obj", "Skybox_Material", "CubeMap"));
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
        mesh->RenderDepth(commandBuffer);
    }
}



void Scene::AlbedoRender(VkCommandBuffer commandBuffer) const
{
	auto mat = MaterialManager::GetMaterial("CompositeMaterial");

	GlobalDescriptor::Bind(ServiceLocator::GetService<VulkanContext>(), commandBuffer, mat->GetPipelineLayout(), PipelineType::Graphics);
	mat->BindPushConstant(commandBuffer, Camera::GetViewMatrix());
	mat->Bind(commandBuffer);
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
		mesh->Render(commandBuffer);

		ImGui::Begin("Mesh");
		if(ImGui::CollapsingHeader(mesh->GetMeshName().c_str()))
		{
			mesh->OnImGui();
		}
		ImGui::End();
	}


    ImGui::Render();
}

void Scene::ExecuteComputePass(VkCommandBuffer commandBuffer) const
{
	auto extends = SwapChain::Extends();
	constexpr uint32_t groupSize = 32;
	uint32_t fullscreenGroupCountX = (extends.width) / groupSize;
	uint32_t fullscreenGroupCountY = (extends.height) / groupSize;

	uint32_t quarterGroupCountX = (extends.width / 2.0f) / groupSize;
	uint32_t quarterGroupCountY = (extends.height / 2.0f) / groupSize;

	Texture* downSampleTexture{};
	Texture* SSAO{};
	Texture* BlurrSSAO{};



	auto downSampleMaterial = MaterialManager::GetMaterial("DownSample");
	if(downSampleMaterial->IsCompute())
	{
		//Transition the output texture to writable
		const auto& textures = downSampleMaterial->GetDescriptorSet()->GetTextures();
		for(const auto& texture : textures)
		{
			if(texture->IsOutputTexture()) downSampleTexture = texture.get();
			break;
		}

		downSampleTexture->TransitionToGeneralImageLayout(commandBuffer);

		downSampleMaterial->Bind(commandBuffer);
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
			if(texture->IsOutputTexture())
			{
				SSAO = texture.get();
				break;
			}
        }

    	SSAO->TransitionToGeneralImageLayout(commandBuffer);

		GlobalDescriptor::Bind(ServiceLocator::GetService<VulkanContext>(), commandBuffer, ssaoMaterial->GetPipelineLayout(), PipelineType::Compute);
    	ssaoMaterial->BindPushConstant(commandBuffer, Camera::GetViewMatrix());
		ssaoMaterial->Bind(commandBuffer);
		vkCmdDispatch(commandBuffer, quarterGroupCountX, quarterGroupCountY	, 1);

    	SSAO->TransitionToReadableImageLayout(commandBuffer);
		SSAO->SetOutputTexture(false);
    }


	auto blurMaterial = MaterialManager::GetMaterial("Blur");
	if(ssaoMaterial->IsCompute())
	{
		const auto& textures = blurMaterial->GetDescriptorSet()->GetTextures();
		//Transition the output texture to writable
		for(const auto& texture : textures)
		{
			if(texture->IsOutputTexture())
			{
				BlurrSSAO = texture.get();
				break;
			}
		}

		BlurrSSAO->TransitionToGeneralImageLayout(commandBuffer);

		blurMaterial->BindPushConstant(commandBuffer, Camera::GetProjectionMatrix());
		blurMaterial->Bind(commandBuffer);
		vkCmdDispatch(commandBuffer, quarterGroupCountX, quarterGroupCountY, 1);

		BlurrSSAO->TransitionToReadableImageLayout(commandBuffer);
		BlurrSSAO->SetOutputTexture(false);
	}




	auto upSampleMaterial = MaterialManager::GetMaterial("UpSample");
	if(upSampleMaterial->IsCompute())
	{
		const auto& textures = upSampleMaterial->GetDescriptorSet()->GetTextures();
		//Transition the output texture to writable
		for(const auto& texture : textures)
		{
			if(texture->IsOutputTexture()) texture->TransitionToGeneralImageLayout(commandBuffer);
		}

		GlobalDescriptor::Bind(ServiceLocator::GetService<VulkanContext>(), commandBuffer, upSampleMaterial->GetPipelineLayout(), PipelineType::Compute);
		upSampleMaterial->BindPushConstant(commandBuffer, Camera::GetProjectionMatrix());
		upSampleMaterial->Bind(commandBuffer);
		vkCmdDispatch(commandBuffer, fullscreenGroupCountX, fullscreenGroupCountY, 1);

		//transition the output texture to readable
			for(const auto& texture : textures)
		{
			if(texture->IsOutputTexture()) texture->TransitionToReadableImageLayout(commandBuffer);
		}
	}

	SSAO->SetOutputTexture(true);
	BlurrSSAO->SetOutputTexture(true);
	downSampleTexture->SetOutputTexture(true);
}

void Scene::CleanUp() const
{
    for (const auto &mesh: m_Meshes)
    {
        mesh->CleanUp();
    }
}
void Scene::AddMesh(std::unique_ptr<Mesh> mesh)
{
    m_Meshes.push_back(std::move(mesh));
}

std::vector<Mesh *> Scene::GetMeshes() const
{
    std::vector<Mesh*> meshes{};
    meshes.reserve(m_Meshes.size());
    for (const auto& mesh : m_Meshes)
    {
        meshes.push_back(mesh.get());
    }
    return meshes;
}