#pragma once
#include <memory>
#include <vector>
#include "Material.h"

class MaterialManager final
{
public:
	MaterialManager() = default;
	~MaterialManager() = default;
	MaterialManager(const MaterialManager&) = delete;
	MaterialManager& operator=(const MaterialManager&) = delete;
	MaterialManager(MaterialManager&&) = delete;
	MaterialManager& operator=(MaterialManager&&) = delete;

	static void OnImGui()
	{
		ImGui::Begin("Materials");
		ImGui::Text("Materials: %d", m_ActiveMaterials.size());
		for (const auto& material : m_ActiveMaterials)
		{
			if(ImGui::CollapsingHeader(material->GetMaterialName().c_str()))
			{
				material->OnImGui();
			}
			
		}
		ImGui::End();
	}

	static std::shared_ptr<Material> CreateMaterial(VulkanContext* vulkanContext, const std::string& materialName)
	{
		m_ActiveMaterials.push_back(std::make_shared<Material>(vulkanContext, materialName));
		return m_ActiveMaterials.back();
	}

	static std::shared_ptr<Material> CreateMaterial(VulkanContext* vulkanContext, const std::string& vertexShaderName, const std::string& fragmentShaderName, const std::string& materialName)
	{
		m_ActiveMaterials.push_back(std::make_shared<Material>(vulkanContext, materialName));

		m_ActiveMaterials.back()->AddShader(vertexShaderName, ShaderType::VertexShader);
		m_ActiveMaterials.back()->AddShader(fragmentShaderName, ShaderType::FragmentShader);

		return m_ActiveMaterials.back();
	}

	static void CreatePipelines()
	{
		for (const auto& material : m_ActiveMaterials)
		{
			material->CreatePipeline();
		}
	}

	static void Cleanup()
	{
		for (const auto& material : m_ActiveMaterials)
		{
			material->CleanUp();
		}
	}

	static void SetCurrentBoundPipeline(GraphicsPipeline* pipeline)
	{
		m_CurrentBoundPipeline = pipeline;
	}

	static GraphicsPipeline* GetCurrentBoundPipeline()
	{
		return m_CurrentBoundPipeline;
	}

private:
	inline static std::vector<std::shared_ptr<Material>> m_ActiveMaterials;

	inline static GraphicsPipeline* m_CurrentBoundPipeline = nullptr;
};