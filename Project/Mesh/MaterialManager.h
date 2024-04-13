#pragma once
#include <memory>
#include <vector>
#include "Material.h"

namespace MaterialManager
{
	inline std::vector<std::shared_ptr<Material>> materials;

	inline std::shared_ptr<Material> CreateMaterial(VulkanContext* vulkanContext) 
	{
		materials.push_back(std::make_shared<Material>(vulkanContext));
		return materials.back();
	}

	inline std::shared_ptr<Material> CreateMaterial(VulkanContext* vulkanContext, const std::string& vertexShaderName, const std::string& fragmentShaderName)
	{
		materials.push_back(std::make_shared<Material>(vulkanContext));

		materials.back()->AddShader(vertexShaderName, ShaderType::VertexShader);
		materials.back()->AddShader(fragmentShaderName, ShaderType::FragmentShader);

		return materials.back();
	}


	inline void CreatePipeline() 
	{
		//Create Pipeline
		for(auto& material : materials) 
		{
			material->CreatePipeline();
		}
	}

	inline void Cleanup() 
	{
		for(auto& material : materials)
		{
			material->CleanUp();	
		}
	}
}