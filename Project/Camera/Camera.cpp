#include "Camera.h"
#include <algorithm>
#include <chrono>
#include <iostream>
#include <ostream>
#include <glm/gtc/matrix_transform.hpp>

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
    constexpr float angularSpeed{ 50.0f * MathConstants::TO_RADIANS };

    // The total movement of this frame
    //glm::vec3 direction{};

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

    m_Pitch = std::clamp(m_Pitch, -89.f * MathConstants::TO_RADIANS, 89.f * MathConstants::TO_RADIANS);


    //Movement calculations
    constexpr float shiftSpeed{ 4.0f };
    //direction *= 1.0f + pKeyboardState[SDL_SCANCODE_LSHIFT] * (shiftSpeed - 1.0f);

    m_Origin += m_Direction * elapsedTime;
    m_Direction = glm::vec3(0.f, 0.f, 0.f);
}

glm::mat4 Camera::GetViewMatrix()
{
    //Calculate the new forward vector with the new pitch and yaw
    //glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), m_Pitch, glm::vec3(1.0f, 0.0f, 0.0f)); // Rotation about X axis
    //rotationMatrix = glm::rotate(rotationMatrix, m_Yaw, glm::vec3(0.0f, 1.0f, 0.0f)); // Rotation about Y axis

   //m_Forward = normalize(glm::vec3(rotationMatrix * glm::vec4(MathConstants::FORWARD,0.f))); // Transforming UnitZ by rotationMatrix
   //m_Right = normalize(glm::cross(MathConstants::FORWARD, m_Forward)); // Taking cross product of UnitY and forward


    const glm::mat4 viewMatrix = glm::lookAt(m_Origin, glm::vec3{0,0,0}, glm::vec3(0.0f, 0.0f, 1.0f));

    return viewMatrix;
}

glm::mat4 Camera::GetProjectionMatrix(float aspectRatio)
{
    glm::mat4x4 projectionMatrix = glm::perspective(m_Fov, aspectRatio, m_NearPlane, m_FarPlane);

    // Flip the Y axis
	projectionMatrix[1][1] *= -1;

    return projectionMatrix;
}

glm::mat4 Camera::GetViewInverseMatrix()
{
	return glm::inverse(GetViewMatrix());
}

glm::mat4 Camera::GetViewProjectionMatrix(float aspectRatio)
{
    return GetProjectionMatrix(aspectRatio) * GetViewMatrix();
}
