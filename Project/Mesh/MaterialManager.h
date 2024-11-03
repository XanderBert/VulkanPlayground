#pragma once
#include <memory>
#include <ranges>
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

	static std::shared_ptr<Material> GetMaterial(const std::string& materialName)
	{
		return m_Materials[materialName];
	}

	static size_t GetSize()
	{
		return m_Materials.size();
	}

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
		ImGui::Text("Materials: %d", static_cast<int>(m_Materials.size()));
		for (const auto& material : m_Materials)
		{
			if(ImGui::CollapsingHeader(material.first.c_str()))
			{
				material.second->OnImGui();
			}
		}
		ImGui::End();
	}

	static std::shared_ptr<Material> CreateMaterial(VulkanContext* vulkanContext, const std::string& materialName)
	{
		m_Materials.insert({materialName, std::make_shared<Material>(vulkanContext, materialName)});
		return m_Materials[materialName];
	}

	static std::shared_ptr<Material> CreateMaterial(VulkanContext* vulkanContext, const std::string& vertexShaderName, const std::string& fragmentShaderName, const std::string& materialName)
	{
		auto back = CreateMaterial(vulkanContext, materialName);

		back->AddShader(vertexShaderName, ShaderType::VertexShader);
		back->AddShader(fragmentShaderName, ShaderType::FragmentShader);

		return back;
	}

	static void CreatePipelines()
	{
		for (const auto &material : m_Materials | std::views::values)
		{
			material->CreatePipeline();
		}

	    LogInfo("Vulkan Pipelines made");
	}

	static void Cleanup()
	{
		for (const auto &material : m_Materials | std::views::values)
		{
			material->CleanUp();
		}
	}

    static std::vector<std::string> GetMaterialNames()
    {
		std::vector<std::string> names = {};
		names.reserve(m_Materials.size());

		for (const auto& material : m_Materials)
		{
			names.push_back(material.first);
		}

		return names;
	}

    static std::vector<std::shared_ptr<Material>> GetMaterials()
    {
		std::vector<std::shared_ptr<Material>> materials = {};
		materials.reserve(m_Materials.size());

		for (const auto& material : m_Materials)
		{
			materials.push_back(material.second);
		}

	    return materials;
	}

    //TODO This needs to be moved to a more appropriate place
    static bool ImGuiMaterialGetter(void* data, int idx, const char** out_text)
    {
		auto names = GetMaterialNames();

		if (idx < 0 || idx >= static_cast<int>(names.size())) return false;

		*out_text = names[idx].c_str();
		return true;
	}

private:
	inline static std::unordered_map<std::string, std::shared_ptr<Material>> m_Materials;
};