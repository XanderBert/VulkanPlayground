#pragma once
#include <glm/vec3.hpp>
#include <glm/matrix.hpp>

#define TO_RADIANS 0.01745329252f

class Camera final
{
public:
    Camera(const glm::vec3& origin, float fov);
    ~Camera() = default;
    Camera(const Camera&) = delete;
    Camera& operator=(const Camera&) = delete;
    Camera(Camera&&) = delete;
    Camera& operator=(Camera&&) = delete;

    void Update(const float elapsedTime);

    glm::mat4 GetViewMatrix();
    glm::mat4 GetProjectionMatrix() const;
    glm::mat4 GetViewInverseMatrix();
    glm::mat4 GetViewProjectionMatrix();

    glm::vec3 GetPosition() const { return m_Origin; }

    void SetFOV(float fov) { m_Fov = fov * (TO_RADIANS * 0.5f); }
    float GetFOV() const { return m_Fov; }


private:
    float m_Fov = 100.f;
    float m_Width = 16.f;
    float m_Height = 9.f;
    glm::vec3 m_Origin = { 0.f, 0.f, 0.f };

    float m_NearPlane = 0.1f;
    float m_FarPlane = 1000.f;

    glm::vec3 m_Forward = { 0.f, 0.f, 1.f };
    glm::vec3 m_Right = { 1.f, 0.f, 0.f };

    float m_Pitch = 0.f;
    float m_Yaw = 0.f;
};