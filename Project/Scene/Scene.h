#pragma once
#include <vector>
#include <memory>
#include "Camera/Camera.h"
#include <vulkan/vulkan.h>
#include "Mesh/Mesh.h"


struct Vertex;


class Scene final
{
public:
	Scene(VulkanContext* vulkanContext);
	~Scene() = default;
	Scene(const Scene&) = delete;
	Scene& operator=(const Scene&) = delete;
	Scene(Scene&&) = delete;
	Scene& operator=(Scene&&) = delete;

    //TODO: A Scene Should store a list of passes
    void RenderDepth(VkCommandBuffer commandBuffer) const;
	void Render(VkCommandBuffer commandBuffer) const;
    void ExecuteComputePass(VkCommandBuffer commandBuffer) const;


	void CleanUp() const;

    void AddMesh(std::unique_ptr<Mesh> mesh);

	[[nodiscard]] std::vector<Mesh*> GetMeshes() const;

private:
	std::vector<std::unique_ptr<Mesh>> m_Meshes{};


};
