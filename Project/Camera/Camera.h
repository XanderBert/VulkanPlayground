#pragma once
#include <glm/vec3.hpp>
#include <glm/matrix.hpp>


namespace MathConstants
{
    constexpr float TO_RADIANS = 0.01745329252f;
    constexpr float TO_HALFRADIANS = 0.00872664626f;

    constexpr float PI = 3.14159265359f;
    constexpr float TWO_PI = 6.28318530718f;
    constexpr float HALF_PI = 1.57079632679f;
    constexpr float INV_PI = 0.31830988618f;

    constexpr glm::vec3 FORWARD = { 0.f, 1.f, 0.f };
    constexpr glm::vec3 RIGHT = { 1.f, 0.f, 0.f };
    constexpr glm::vec3 UP = { 0.f, 0.f, 1.f };
}



class Camera final
{
public:
    Camera() = default;
    ~Camera() = default;
    Camera(const Camera&) = delete;
    Camera& operator=(const Camera&) = delete;
    Camera(Camera&&) = delete;
    Camera& operator=(Camera&&) = delete;

    static void Update(const float elapsedTime);

    static glm::mat4 GetViewMatrix();
    static glm::mat4 GetProjectionMatrix(float aspectRatio);
    static glm::mat4 GetViewInverseMatrix();
    static glm::mat4 GetViewProjectionMatrix(float aspectRatio);

    static glm::vec3 GetPosition() { return m_Origin; }

    static void MoveForward() { m_Direction += m_Forward * m_KeyboardMovementSpeed; }
    static void MoveBackward() { m_Direction -= m_Forward * m_KeyboardMovementSpeed; }
    static void MoveRight() { m_Direction += m_Right * m_KeyboardMovementSpeed; }
    static void MoveLeft() { m_Direction -= m_Right * m_KeyboardMovementSpeed; }

    static void SetFOV(float fov) { m_Fov = fov * MathConstants::TO_HALFRADIANS; }
    static float GetFOV() { return m_Fov / MathConstants::TO_HALFRADIANS; }

private:
    inline static glm::vec3 m_Direction = { 0.f, 0.f, 0.f };



    inline static float m_Fov = 100.f * MathConstants::TO_HALFRADIANS;
    inline static float m_Width = 16.f;
    inline static float m_Height = 9.f;
    inline static  glm::vec3 m_Origin = { 2.f, 2.f, 2.0f };

    inline static float m_NearPlane = 0.1f;
    inline static float m_FarPlane = 1000.f;

    inline static glm::vec3 m_Forward = { 0.f, 1.f, 0.f };
    inline static glm::vec3 m_Right = { 1.f, 0.f, 0.f };

    inline static float m_Pitch = 0.f;
    inline static float m_Yaw = 0.f;

    inline static float m_KeyboardMovementSpeed = 10.f;
};