#include "Mesh.h"
#include <chrono>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <utility>

#include "Camera/Camera.h"
#include "Core/GlobalDescriptor.h"
#include "Core/ImGuiWrapper.h"
#include "ImGuizmo.h"
#include "MaterialManager.h"
#include "ModelLoader.h"
#include "Patterns/ServiceLocator.h"
#include "Timer/GameTimer.h"
#include "Vertex.h"
#include "Core/Logger.h"
#include "vulkanbase/VulkanTypes.h"


// Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices,
//            const std::shared_ptr<Material> &material, const std::shared_ptr<Material> &depthMaterial, std::string  meshName, uint32_t firstDrawIndex, int32_t vertexOffset)
// 	: m_FirstDrawIndex(firstDrawIndex)
//     , m_VertexOffset(vertexOffset)
// 	, m_pMaterial(material)
//     , m_pDepthMaterial(depthMaterial)
//     , m_MeshName(std::move(meshName))
// {
// 	m_pContext = ServiceLocator::GetService<VulkanContext>();
// 	CreateVertexBuffer(vertices);
// 	CreateIndexBuffer(indices);
// }




Mesh::Mesh(const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices, std::string meshName, const std::vector<Primitive> &primitives, uint32_t firstDrawIndex, int32_t vertexOffset)
	: m_FirstDrawIndex(firstDrawIndex)
	, m_VertexOffset(vertexOffset)
	, m_Primitives(primitives)
	, m_pDepthMaterial(MaterialManager::GetMaterial("DepthOnlyMaterial"))
	, m_MeshName(std::move(meshName))
{
	m_pContext = ServiceLocator::GetService<VulkanContext>();

	m_IndexCount = indices.size();

	CreateVertexBuffer(vertices);
	CreateIndexBuffer(indices);
}


Mesh::Mesh(const std::string& modelPath,const std::string& materialName, const std::string& meshName)
	: m_pDepthMaterial(MaterialManager::GetMaterial("DepthOnlyMaterial"))
	, m_MeshName(std::move(meshName))
{
	m_pContext = ServiceLocator::GetService<VulkanContext>();
	m_MeshName = meshName.empty() ? m_MeshName = modelPath : m_MeshName = meshName;

	//Load the .obj file
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	ObjLoader::LoadObj(modelPath, vertices, indices);

	Primitive primitive{};
	primitive.firstIndex = 0;
	primitive.indexCount = static_cast<uint32_t>(indices.size());
	primitive.material = MaterialManager::GetMaterial(materialName);
	m_Primitives.push_back(primitive);

	CreateVertexBuffer(vertices);
	CreateIndexBuffer(indices);
}


void Mesh::Render(VkCommandBuffer commandBuffer)
{
	if(m_Rotate)
	m_ModelMatrix = glm::rotate(m_ModelMatrix, GameTimer::GetDeltaTime() * glm::radians(m_RotationSpeed), MathConstants::UP);

	m_Visible = m_VisibleBuffer;
	if (!m_Visible) return;

	m_IndexBuffer.BindAsIndexBuffer(commandBuffer);
	m_VertexBuffer.BindAsVertexBuffer(commandBuffer);

	for(const auto& primitive: m_Primitives)
	{
		GlobalDescriptor::Bind(m_pContext, commandBuffer, primitive.material->GetPipelineLayout());
		primitive.Render(commandBuffer, m_ModelMatrix);
	}
}

void Mesh::RenderDepth(VkCommandBuffer commandBuffer)
{
    if (!m_Visible) return;

    GlobalDescriptor::Bind(m_pContext, commandBuffer, m_pDepthMaterial->GetPipelineLayout());
    m_pDepthMaterial->Bind(commandBuffer, m_ModelMatrix);
	m_IndexBuffer.BindAsIndexBuffer(commandBuffer);
	m_VertexBuffer.BindAsVertexBuffer(commandBuffer);
	vkCmdDrawIndexed(commandBuffer, m_IndexCount, 1, m_FirstDrawIndex, m_VertexOffset, 0);
}


void Mesh::OnImGui()
{
	const std::string labelAddition = "##" + std::to_string(reinterpret_cast<uint64_t>(this));

	ImGui::Indent();
	const std::string Infolabel = "Info" + labelAddition;
	if(ImGui::CollapsingHeader(Infolabel.c_str()))
	{
		ImGui::Text("Mesh Name: %s", m_MeshName.c_str());
		//ImGui::Text("Vertex Count: %d", m_VertexCount);
		ImGui::Text("Index Count: %d", m_IndexCount);
	    //ImGui::Text("Active Material: %s", m_pMaterial->GetMaterialName().c_str());

		//TODO: Fix this
        //Get Dropdown menu with all material names
	    // auto materials = MaterialManager::GetMaterials();
     //    int selectedMaterialIndex = -1;
	    // if(ImGui::ListBox("##Mateial:",&selectedMaterialIndex , MaterialManager::ImGuiMaterialGetter, materials.data(), materials.size()))
	    // {
	    //     m_pMaterial = materials[selectedMaterialIndex];
     //
	    //     std::string log = "Material changed to: " + m_pMaterial->GetMaterialName() + "For Mesh: " + m_MeshName;
	    //     LogInfo(log);
	    // }
	}

	ImGui::Unindent();
	ImGui::Separator();
	const std::string visibilityLabel = "Visible" + labelAddition;
	ImGui::Checkbox(visibilityLabel.c_str(), &m_VisibleBuffer);

	const std::string rotateLabel = "Rotate" + labelAddition;
	ImGui::Checkbox(rotateLabel.c_str(), &m_Rotate);

	const std::string rotationSpeedLabel = "Rotation Speed" + labelAddition;
	ImGui::DragFloat(rotationSpeedLabel.c_str(), &m_RotationSpeed);
	

	ImGui::Separator();
	ImGui::Text("Model Matrix: ");
	float matrixTranslation[3], matrixRotation[3], matrixScale[3];
	const std::string translationLabel = "Translation" + labelAddition;
	const std::string rotationLabel = "Rotation" + labelAddition;
	const std::string scaleLabel = "Scale" + labelAddition;

	ImGuizmo::DecomposeMatrixToComponents(value_ptr(m_ModelMatrix), matrixTranslation, matrixRotation, matrixScale);
	ImGui::DragFloat3(translationLabel.c_str(), matrixTranslation, 0.1f);
	ImGui::DragFloat3(rotationLabel.c_str(), matrixRotation, 0.1f);
	ImGui::DragFloat3(scaleLabel.c_str(), matrixScale, 0.1f);
	ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, glm::value_ptr(m_ModelMatrix));

    //Guizmos
    ImGuizmo::Manipulate(value_ptr(Camera::GetViewMatrix()), value_ptr(Camera::GetInvertedYProjectionMatrix()), ImGuizmoHandler::GizmoOperation, ImGuizmo::MODE:: LOCAL, value_ptr(m_ModelMatrix));
}


void Mesh::CleanUp()
{
	m_IndexBuffer.Cleanup();
	m_VertexBuffer.Cleanup();
}

void Mesh::SetPosition(const glm::vec3& position)
{
	m_ModelMatrix = glm::translate(glm::mat4(1.0f), position);
}

void Mesh::SetScale(const glm::vec3& scale)
{
	m_ModelMatrix = glm::scale(m_ModelMatrix, scale);
}


void Mesh::SetRotation(const glm::vec3 &rotation) {
    m_ModelMatrix = glm::rotate(m_ModelMatrix, glm::radians(rotation.x), MathConstants::RIGHT);
    m_ModelMatrix = glm::rotate(m_ModelMatrix, glm::radians(rotation.y), MathConstants::UP);
    m_ModelMatrix = glm::rotate(m_ModelMatrix, glm::radians(rotation.z), MathConstants::FORWARD);
}

void Mesh::CreateVertexBuffer(const std::vector<Vertex>& vertices)
{
	//Store the vertex count
	m_VertexBuffer.count = static_cast<uint32_t>(vertices.size());


	//Check if the mesh has at least 3 vertices
	LogAssert(m_VertexBuffer.count >= 3,"Mesh has less then 3 vertices", false);

	//Get the buffer size
	const VkDeviceSize bufferSize = sizeof(vertices[0]) * (m_VertexBuffer.count);

	//Create a staging buffer
	VkBuffer stagingBuffer;
	VmaAllocation stagingBufferMemory;
	Core::Buffer::CreateStagingBuffer<Vertex>(bufferSize, stagingBuffer, stagingBufferMemory, vertices.data());


	//Create a Vertex buffer
	Core::Buffer::CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, m_VertexBuffer.buffer, m_VertexBuffer.bufferMemory);

	//Copy the staging buffer to the vertex buffer
	Core::Buffer::CopyBuffer(m_pContext, stagingBuffer, m_VertexBuffer.buffer, bufferSize);
    vmaDestroyBuffer(Allocator::vmaAllocator, stagingBuffer, stagingBufferMemory);
}

void Mesh::CreateIndexBuffer(const std::vector<uint32_t>& indices)
{
	m_IndexBuffer.count = indices.size();
	LogAssert(m_IndexBuffer.count >= 3, "Mesh has less then 3 indices", false);

	//Get the buffer size
	const VkDeviceSize bufferSize = sizeof(indices[0]) * (m_IndexBuffer.count);

	//Create a staging buffer
	VkBuffer stagingBuffer;
	VmaAllocation stagingBufferMemory;
	Core::Buffer::CreateStagingBuffer<uint32_t>(bufferSize, stagingBuffer, stagingBufferMemory, indices.data());


	//Create A index buffer
	Core::Buffer::CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, m_IndexBuffer.buffer, m_IndexBuffer.bufferMemory);

	//Copy the staging buffer to the index buffer
	Core::Buffer::CopyBuffer(m_pContext, stagingBuffer, m_IndexBuffer.buffer, bufferSize);
    vmaDestroyBuffer(Allocator::vmaAllocator, stagingBuffer, stagingBufferMemory);
}