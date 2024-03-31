#include "Camera.h"

#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>

Camera::Camera(const glm::vec3& origin, float fov):
	m_Fov(fov* (TO_RADIANS * 0.5f)),
	m_Origin(origin)
{}

void Camera::Update(const float elapsedTime)
{
    // Keyboard Input
    //const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);

    // Mouse Input
    //int mouseX{}, mouseY{};
    //const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);

    // Speed and limit constants
    constexpr float keyboardMovementSpeed{ 10.0f };
    constexpr float mouseMovementSpeed{ 2.0f };
    constexpr float angularSpeed{ 50.0f * TO_RADIANS };

    // The total movement of this frame
    glm::vec3 direction{};

    // Calculate new position with keyboard inputs
    //direction += (pKeyboardState[SDL_SCANCODE_W] || pKeyboardState[SDL_SCANCODE_Z]) * m_Forward * keyboardMovementSpeed * deltaTime;
    //direction -= pKeyboardState[SDL_SCANCODE_S] * m_Forward * keyboardMovementSpeed * deltaTime;
    //direction -= (pKeyboardState[SDL_SCANCODE_Q] || pKeyboardState[SDL_SCANCODE_A]) * m_Right * keyboardMovementSpeed * deltaTime;
    //direction += pKeyboardState[SDL_SCANCODE_D] * m_Right * keyboardMovementSpeed * deltaTime;

#ifndef IMGUI_DISABLE  
    //if (!ImGui::GetIO().WantCaptureMouse)
#endif

	// Calculate new position and rotation with mouse inputs
    {
        //switch (mouseState)
        //{
        //case SDL_BUTTON_LMASK: // LEFT CLICK
        //    direction -= m_Forward * (mouseY * mouseMovementSpeed * deltaTime);
        //    m_Yaw += mouseX * angularSpeed * deltaTime;
        //    break;
        //case SDL_BUTTON_RMASK: // RIGHT CLICK
        //    m_Yaw += mouseX * angularSpeed * deltaTime;
        //    m_Pitch -= mouseY * angularSpeed * deltaTime;
        //    break;
        //case SDL_BUTTON_X2: // BOTH CLICK
        //    direction.y -= mouseY * mouseMovementSpeed * deltaTime;
        //    break;
        //}
    }

    m_Pitch = std::clamp(m_Pitch, -89.f * TO_RADIANS, 89.f * TO_RADIANS);


    //Movement calculations
    constexpr float shiftSpeed{ 4.0f };
    //direction *= 1.0f + pKeyboardState[SDL_SCANCODE_LSHIFT] * (shiftSpeed - 1.0f);

    m_Origin += direction;
}

glm::mat4 Camera::GetViewMatrix()
{
    //Calculate the new forward vector with the new pitch and yaw
    glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), m_Pitch, glm::vec3(1.0f, 0.0f, 0.0f)); // Rotation about X axis
    rotationMatrix = glm::rotate(rotationMatrix, m_Yaw, glm::vec3(0.0f, 1.0f, 0.0f)); // Rotation about Y axis


    m_Forward = normalize(glm::vec3(rotationMatrix * glm::vec4(0.0f, 0.0f, 1.0f, 0.0f))); // Transforming UnitZ by rotationMatrix
    m_Right = normalize(glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), m_Forward)); // Taking cross product of UnitY and forward


    const glm::mat4 lookAtMatrix = glm::lookAtLH(m_Origin, m_Forward, m_Right);


    //Return the inverse of the inverseViewMatrix
    return glm::inverse(lookAtMatrix);
}

glm::mat4 Camera::GetProjectionMatrix() const
{
    //depth = [0, 1] (Direct3D Depth)
    return glm::perspectiveFovLH(m_Fov, m_Width, m_Height, m_NearPlane, m_FarPlane);
}

glm::mat4 Camera::GetViewInverseMatrix()
{
	return glm::inverse(GetViewMatrix());
}

glm::mat4 Camera::GetViewProjectionMatrix()
{
	return GetViewMatrix() * GetProjectionMatrix();
}
