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

	static std::shared_ptr<Material> CreateMaterial(VulkanContext* vulkanContext)
	{
		m_ActiveMaterials.push_back(std::make_shared<Material>(vulkanContext));
		return m_ActiveMaterials.back();
	}

	static std::shared_ptr<Material> CreateMaterial(VulkanContext* vulkanContext, const std::string& vertexShaderName, const std::string& fragmentShaderName)
	{
		m_ActiveMaterials.push_back(std::make_shared<Material>(vulkanContext));

		m_ActiveMaterials.back()->AddShader(vertexShaderName, ShaderType::VertexShader);
		m_ActiveMaterials.back()->AddShader(fragmentShaderName, ShaderType::FragmentShader);

		return m_ActiveMaterials.back();
	}

	static void CreatePipeline()
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

private:
	inline static std::vector<std::shared_ptr<Material>> m_ActiveMaterials;
};