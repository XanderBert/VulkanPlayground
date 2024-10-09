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


Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices,
           const std::shared_ptr<Material> &material, const std::shared_ptr<Material> &depthMaterial, std::string  meshName, uint32_t firstDrawIndex, int32_t vertexOffset)
	: m_FirstDrawIndex(firstDrawIndex)
    , m_VertexOffset(vertexOffset)
	, m_pMaterial(material)
    , m_pDepthMaterial(depthMaterial)
    , m_MeshName(std::move(meshName))
{
	m_pContext = ServiceLocator::GetService<VulkanContext>();
	CreateVertexBuffer(vertices);
	CreateIndexBuffer(indices);
}

Mesh::Mesh(const std::string& modelPath, const std::shared_ptr<Material> &material, const std::shared_ptr<Material> &depthMaterial,  const std::string& meshName)
	:m_pMaterial(material)
    ,m_pDepthMaterial(depthMaterial)
{
	m_MeshName = meshName.empty() ? m_MeshName = modelPath : m_MeshName = meshName;

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

    //TODO: also support gltf in this function
	ObjLoader::LoadObj(modelPath, vertices, indices);

	m_pContext = ServiceLocator::GetService<VulkanContext>();

	CreateVertexBuffer(vertices);
	CreateIndexBuffer(indices);
}


void Mesh::Bind(VkCommandBuffer commandBuffer)
{
	if(m_Rotate)
	m_ModelMatrix = glm::rotate(m_ModelMatrix, GameTimer::GetDeltaTime() * glm::radians(m_RotationSpeed), MathConstants::UP);

	m_Visible = m_VisibleBuffer;
	if (!m_Visible) return;

    GlobalDescriptor::Bind(m_pContext, commandBuffer, m_pMaterial->GetPipelineLayout());
	m_pMaterial->Bind(commandBuffer, m_ModelMatrix);

	const VkBuffer vertexBuffers[] = { m_VertexBuffer };
	constexpr VkDeviceSize offsets[] = { 0 }; 

	vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
	vkCmdBindIndexBuffer(commandBuffer, m_IndexBuffer, 0, VK_INDEX_TYPE_UINT32);
}

void Mesh::BindDepth(VkCommandBuffer commandBuffer)
{
    if (!m_Visible) return;

    GlobalDescriptor::Bind(m_pContext, commandBuffer, m_pDepthMaterial->GetPipelineLayout());
    m_pDepthMaterial->Bind(commandBuffer, m_ModelMatrix);

    const VkBuffer vertexBuffers[] = { m_VertexBuffer };
    constexpr VkDeviceSize offsets[] = { 0 };

    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(commandBuffer, m_IndexBuffer, 0, VK_INDEX_TYPE_UINT32);
}


void Mesh::OnImGui()
{
	const std::string labelAddition = "##" + std::to_string(reinterpret_cast<uint64_t>(this));

	ImGui::Indent();
	const std::string Infolabel = "Info" + labelAddition;
	if(ImGui::CollapsingHeader(Infolabel.c_str()))
	{
		ImGui::Text("Mesh Name: %s", m_MeshName.c_str());
		ImGui::Text("Vertex Count: %d", m_VertexCount);
		ImGui::Text("Index Count: %d", m_IndexCount);
	    ImGui::Text("Active Material: %s", m_pMaterial->GetMaterialName().c_str());

        //Get Dropdown menu with all material names
	    auto materials = MaterialManager::GetMaterials();
        int selectedMaterialIndex = -1;
	    if(ImGui::ListBox("##Mateial:",&selectedMaterialIndex , MaterialManager::ImGuiMaterialGetter, materials.data(), materials.size()))
	    {
	        m_pMaterial = materials[selectedMaterialIndex];

	        std::string log = "Material changed to: " + m_pMaterial->GetMaterialName() + "For Mesh: " + m_MeshName;
	        LogInfo(log);
	    }
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


void Mesh::Render(VkCommandBuffer commandBuffer)
{
	if (!m_Visible) return;
	vkCmdDrawIndexed(commandBuffer, m_IndexCount, 1, m_FirstDrawIndex, m_VertexOffset, 0);
}

void Mesh::CleanUp()
{
    vmaDestroyBuffer(Allocator::VmaAllocator, m_VertexBuffer, m_VertexBufferMemory);
    vmaDestroyBuffer(Allocator::VmaAllocator, m_IndexBuffer, m_IndexBufferMemory);
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
	m_VertexCount = static_cast<uint32_t>(vertices.size());

	//Check if the mesh has at least 3 vertices
	LogAssert(m_VertexCount >= 3,"Mesh has less then 3 vertices", false);

	//Get the device and the buffer size
	const VkDevice device = m_pContext->device;
	const VkDeviceSize bufferSize = sizeof(vertices[0]) * (m_VertexCount);

	//Create a staging buffer
	VkBuffer stagingBuffer;
	VmaAllocation stagingBufferMemory;
	Core::Buffer::CreateStagingBuffer<Vertex>(bufferSize, stagingBuffer, stagingBufferMemory, vertices.data());


	//Create a Vertex buffer
	Core::Buffer::CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, m_VertexBuffer, m_VertexBufferMemory);

	//Copy the staging buffer to the vertex buffer
	Core::Buffer::CopyBuffer(m_pContext, stagingBuffer, m_VertexBuffer, bufferSize);
    vmaDestroyBuffer(Allocator::VmaAllocator, stagingBuffer, stagingBufferMemory);
}

void Mesh::CreateIndexBuffer(const std::vector<uint32_t>& indices)
{
	m_IndexCount = indices.size();
	LogAssert(m_IndexCount >= 3, "Mesh has less then 3 indices", false);

	//Get the device and the buffer size
	const VkDevice device = m_pContext->device;
	const VkDeviceSize bufferSize = sizeof(indices[0]) * (m_IndexCount);

	//Create a staging buffer
	VkBuffer stagingBuffer;
	VmaAllocation stagingBufferMemory;
	Core::Buffer::CreateStagingBuffer<uint32_t>(bufferSize, stagingBuffer, stagingBufferMemory, indices.data());


	//Create A index buffer
	Core::Buffer::CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, m_IndexBuffer, m_IndexBufferMemory);

	//Copy the staging buffer to the index buffer
	Core::Buffer::CopyBuffer(m_pContext, stagingBuffer, m_IndexBuffer, bufferSize);
    vmaDestroyBuffer(Allocator::VmaAllocator, stagingBuffer, stagingBufferMemory);
}