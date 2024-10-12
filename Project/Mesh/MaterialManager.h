#pragma once
#include <memory>
#include <vector>
#include "Material.h"
#include "Patterns/ServiceLocator.h"
#include "shaders/Logic/Shader.h"

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
        ImGui::Begin("Material Factory");

	        if(ImGui::Button("New Material"))
	        {
	            auto* pContext = ServiceLocator::GetService<VulkanContext>();

	            auto material = CreateMaterial(pContext, "shader.vert", "shader.frag", "New Test Material");

	            //TODO: In a perfect world, I would read the shader files and look at set 1. Add all the needed bindings to the descriptor set
                //TODO: This would actually also be done when a shader gets reloaded because bindings could be added or removed
                //Actual data can just be an imgui widget that can be added to the descriptor set.
	            //A texture will for now just be grey. But textures would need to be switched out when clicked on it.

	            //Add the needed bindings for the descriptorset
	            material->GetDescriptorSet()->AddTexture(1, "vehicle_normal.png", pContext);
	            material->GetDescriptorSet()->AddTexture(2, "vehicle_normal.png", pContext);
	            material->GetDescriptorSet()->AddTexture(3, "vehicle_normal.png", pContext);
	            material->GetDescriptorSet()->AddTexture(4, "vehicle_normal.png", pContext);

	            //Create Pipeline
                material->CreatePipeline();
	        }

	    ImGui::End();


		ImGui::Begin("Active Materials");
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
};