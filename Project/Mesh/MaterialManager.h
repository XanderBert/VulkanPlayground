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
		ImGui::Text("Materials: %d", static_cast<int>(m_ActiveMaterials.size()));
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
	    m_MaterialNames.push_back(materialName);
		return m_ActiveMaterials.back();
	}

	static std::shared_ptr<Material> CreateMaterial(VulkanContext* vulkanContext, const std::string& vertexShaderName, const std::string& fragmentShaderName, const std::string& materialName)
	{
		m_ActiveMaterials.push_back(std::make_shared<Material>(vulkanContext, materialName));

		m_ActiveMaterials.back()->AddShader(vertexShaderName, ShaderType::VertexShader);
		m_ActiveMaterials.back()->AddShader(fragmentShaderName, ShaderType::FragmentShader);

	    m_MaterialNames.push_back(materialName);
		return m_ActiveMaterials.back();
	}

	static void CreatePipelines()
	{
		for (const auto& material : m_ActiveMaterials)
		{
			material->CreatePipeline();
		}

	    LogInfo("Vulkan Pipelines made");
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

    static std::vector<std::string> GetMaterialNames()
    {
	    return m_MaterialNames;
	}

    static std::vector<std::shared_ptr<Material>> GetMaterials()
    {
	    return m_ActiveMaterials;
	}

    //TODO This needs to be moved to a more appropriate place
    static bool ImGuiMaterialGetter(void* data, int idx, const char** out_text)
    {
	    *out_text = m_MaterialNames[idx].c_str();
	    return true;
	}

private:
    inline static std::vector<std::string> m_MaterialNames;
	inline static std::vector<std::shared_ptr<Material>> m_ActiveMaterials;
	inline static GraphicsPipeline* m_CurrentBoundPipeline = nullptr;
};