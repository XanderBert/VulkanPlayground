#include "Camera.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Core/Logger.h"
#include "Core/SwapChain.h"
#include "Input/Input.h"
#include "Timer/GameTimer.h"
#include "glm/gtx/optimum_pow.hpp"


void Camera::Init()
{
    SwapChain::OnSwapChainRecreated.AddLambda([&](const VulkanContext* context)
    {
        const auto swapChainExtent = SwapChain::Extends();
        SetAspectRatio(static_cast<float>(swapChainExtent.width), static_cast<float>(swapChainExtent.height));
    });
}

float Camera::GetNearPlane() { return m_NearPlane; }
float Camera::GetFarPlane() { return m_FarPlane; }
glm::mat4 Camera::GetViewMatrix()
{
    return glm::lookAt(m_Origin, m_Origin + m_Forward, m_Up);
}

glm::mat4 Camera::GetInvertedYProjectionMatrix()
{
    return  glm::perspective(m_Fov, m_Width / m_Height, m_NearPlane, m_FarPlane);
}

glm::mat4 Camera::GetProjectionMatrix()
{
    glm::mat4x4 projectionMatrix = glm::perspective(m_Fov, m_Width / m_Height, m_NearPlane, m_FarPlane);

    // Flip the Y axis
	projectionMatrix[1][1] *= -1;

    return projectionMatrix;
}

glm::mat4 Camera::GetViewInverseMatrix()
{
	return glm::inverse(GetViewMatrix());
}

glm::mat4 Camera::GetViewProjectionMatrix()
{
    return GetProjectionMatrix() * GetViewMatrix();
}

void Camera::MoveForward()
{
	const glm::vec3 moveDirection = m_Forward * m_KeyboardMovementSpeed;
    m_Target += moveDirection;
}

void Camera::MoveBackward()
{
	const glm::vec3 moveDirection = -m_Forward * m_KeyboardMovementSpeed;
    m_Target += moveDirection;
}

void Camera::MoveRight()
{
	const glm::vec3 moveDirection = m_Right * m_KeyboardMovementSpeed;
    m_Target += moveDirection;
}

void Camera::MoveLeft()
{
	const glm::vec3 moveDirection = -m_Right * m_KeyboardMovementSpeed;
    m_Target += moveDirection;
}

void Camera::OnRightPressed()
{
	m_DragStartPos = Input::GetMousePosition();
}

void Camera::OnMouseMoved(glm::vec2 mousePos)
{
	if(!Input::IsMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT)) return;

	const glm::vec2 delta = mousePos - m_DragStartPos;

	//Horizontal rotation
	float pitch{};
	if (delta.x > 0) pitch -= m_AngularMovementSpeed * GameTimer::GetDeltaTime();
	else if (delta.x < 0) pitch += m_AngularMovementSpeed * GameTimer::GetDeltaTime();

	// Clamp pitch to avoid gimbal lock
	pitch = std::clamp(pitch, -89.f * MathConstants::TO_RADIANS, 89.f * MathConstants::TO_RADIANS);


	//Vertical rotation
	float yaw{};
	if (delta.y > 0) yaw += m_AngularMovementSpeed * GameTimer::GetDeltaTime();
	else if (delta.y < 0) yaw -= m_AngularMovementSpeed * GameTimer::GetDeltaTime();

	//Apply rotation
	glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), pitch, m_Up);
	rotationMatrix = glm::rotate(rotationMatrix, yaw, m_Right);

	//Recalculate the forward, right and up vectors
	m_Forward = glm::normalize(glm::vec3(rotationMatrix * glm::vec4(m_Forward, 0.f)));
	m_Right = glm::normalize(glm::cross(MathConstants::UP, m_Forward));
	m_Up = glm::normalize(glm::cross(m_Forward, m_Right));


	//Update the drag start position
	m_DragStartPos = mousePos;
}

void Camera::SetFOV(float fov)
{
	m_Fov = fov * MathConstants::TO_HALFRADIANS;
}

float Camera::GetFOV()
{
	return m_Fov / MathConstants::TO_HALFRADIANS;
}

void Camera::OnImGui()
{
    ImGui::Begin("Camera Settings");
    ImGui::Text("Camera");

    if (ImGui::DragFloat("FOV", &m_Fov, 0.1f))
    {
        m_Fov = std::clamp(m_Fov / MathConstants::TO_HALFRADIANS, 40.f, 220.f) * MathConstants::TO_HALFRADIANS;
    }

    if(ImGui::DragFloat3("Position", glm::value_ptr(m_Origin), 0.1f))
    {
        m_Target = m_Origin;
    }

    ImGui::DragFloat("Movement Speed", &m_KeyboardMovementSpeed, 0.1f);
    ImGui::DragFloat("Angular Speed", &m_AngularMovementSpeed, 0.1f);
    ImGui::DragFloat("Near Plane", &m_NearPlane, 0.5f);
    ImGui::DragFloat("Far Plane", &m_FarPlane, 0.5f);
    ImGui::DragFloat("Smoothing", &m_MovementSmoothness, 0.01f);

    ImGui::End();
}
void Camera::Update()
{
    //Smoothly move the camera to the target
    m_Origin = m_Target +  (m_Origin - m_Target) * glm::pow3((-GameTimer::GetDeltaTime() / m_MovementSmoothness));
}

void Camera::SetAspectRatio(float width, float height)
{
    m_Width = width;
    m_Height = height;
}
