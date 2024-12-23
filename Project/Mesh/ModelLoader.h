#pragma once
#include <cstdint>

#include <string>
#include <vector>

#undef min
#undef max
#include <fastgltf/types.hpp>

#include "Mesh.h"



class Scene;
struct Vertex;

// namespace ModelMath
// {
//
// }

namespace ObjLoader
{
	void LoadObj(const std::string& filePath, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);
}

namespace GLTFLoader
{
    void LoadGLTF(std::string_view filePath, Scene* scene, VulkanContext *vulkanContext);
	//inline static std::vector<std::string> m_CreatedMaterialNames;

    std::optional<fastgltf::Asset> Load(std::string_view filePath);
    void CreateMaterials(const fastgltf::Asset& gltf, VulkanContext* vulkanContext,std::vector<std::string>& createdMaterialNames);
	void LoadImage(const fastgltf::Image& image,std::vector<std::variant<std::filesystem::path, ImageInMemory>>& images);
	glm::mat4 ComputeTransformMatrix(const fastgltf::TRS& trs);

    VkFilter GetVkFilter(fastgltf::Filter filter);
    VkSamplerMipmapMode GetVkSamplerMipmapMode(fastgltf::Filter mode);
}