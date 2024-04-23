#include "Mesh.h"
#include <chrono>

#include "ImGuizmo.h"
#include "ModelLoader.h"
#include "vulkanbase/VulkanTypes.h"
#include "Vertex.h"
#include "Camera/Camera.h"
#include "Patterns/ServiceLocator.h"
#include "Timer/GameTimer.h"


Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, std::shared_ptr<Material> material, const std::string& meshName)
	:m_pMaterial(material)
	, m_MeshName(meshName)
{
	m_pContext = ServiceLocator::GetService<VulkanContext>();
	CreateVertexBuffer(vertices);
	CreateIndexBuffer(indices);
}

Mesh::Mesh(const std::string& modelPath, std::shared_ptr<Material> material, const std::string& meshName)
	:m_pMaterial(material)
{
	m_MeshName = meshName.empty() ? m_MeshName = modelPath : m_MeshName = meshName;

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

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

	m_pMaterial->Bind(commandBuffer, m_ModelMatrix);

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
		ImGui::Text("Material: %s", m_pMaterial->GetMaterialName().c_str());

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
	ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, value_ptr(m_ModelMatrix));
}


void Mesh::Render(VkCommandBuffer commandBuffer)
{
	if (!m_Visible) return;
	vkCmdDrawIndexed(commandBuffer, m_IndexCount, 1, 0, 0, 0);
}

void Mesh::CleanUp() const
{
	vkDestroyBuffer(m_pContext->device, m_VertexBuffer, nullptr);
	vkFreeMemory(m_pContext->device, m_VertexBufferMemory, nullptr);

	vkDestroyBuffer(m_pContext->device, m_IndexBuffer, nullptr);
	vkFreeMemory(m_pContext->device, m_IndexBufferMemory, nullptr);
}

void Mesh::SetPosition(const glm::vec3& position)
{
	m_ModelMatrix = glm::translate(glm::mat4(1.0f), position);
}

void Mesh::SetScale(const glm::vec3& scale)
{
	m_ModelMatrix = glm::scale(m_ModelMatrix, scale);
}


void Mesh::SetRotation(const glm::vec3& rotation)
{
	m_ModelMatrix = glm::rotate(m_ModelMatrix, glm::radians(rotation.x), MathConstants::RIGHT);
	m_ModelMatrix = glm::rotate(m_ModelMatrix, glm::radians(rotation.y), MathConstants::UP);
	m_ModelMatrix = glm::rotate(m_ModelMatrix, glm::radians(rotation.z), MathConstants::FORWARD);
}

void Mesh::CreateVertexBuffer(const std::vector<Vertex>& vertices)
{
	//Store the vertex count
	m_VertexCount = static_cast<uint16_t>(vertices.size());

	//Check if the mesh has at least 3 vertices
	LogAssert(m_VertexCount >= 3,"Mesh has less then 3 vertices", false);

	//Get the device and the buffer size
	const VkDevice device = m_pContext->device;
	const VkDeviceSize bufferSize = sizeof(vertices[0]) * m_VertexCount;

	//Create a staging buffer
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;


	Core::Buffer::CreateStagingBuffer<Vertex>(m_pContext, bufferSize, stagingBuffer, stagingBufferMemory, vertices.data());


	//Create a Vertex buffer
	Core::Buffer::CreateBuffer(m_pContext, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_VertexBuffer, m_VertexBufferMemory);

	//Copy the staging buffer to the vertex buffer
	Core::Buffer::CopyBuffer(m_pContext, stagingBuffer, m_VertexBuffer, bufferSize);


	//Clean up the staging buffer
	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void Mesh::CreateIndexBuffer(const std::vector<uint32_t>& indices)
{
	m_IndexCount = static_cast<uint16_t>(indices.size());
	LogAssert(m_IndexCount >= 3, "Mesh has less then 3 indices", false);


	//Get the device and the buffer size
	const VkDevice device = m_pContext->device;
	const VkDeviceSize bufferSize = sizeof(indices[0]) * m_IndexCount;

	auto data = indices.data();

	//Create a staging buffer
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	Core::Buffer::CreateStagingBuffer<uint32_t>(m_pContext, bufferSize, stagingBuffer, stagingBufferMemory, indices.data());


	//Create A index buffer
	Core::Buffer::CreateBuffer(m_pContext, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_IndexBuffer, m_IndexBufferMemory);

	//Copy the staging buffer to the index buffer
	Core::Buffer::CopyBuffer(m_pContext, stagingBuffer, m_IndexBuffer, bufferSize);

	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);
}