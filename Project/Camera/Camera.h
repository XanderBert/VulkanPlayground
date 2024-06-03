#pragma once
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

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

    static void Init();

    static float GetNearPlane();
    static float GetFarPlane();

    static glm::mat4 GetViewMatrix();
    static glm::mat4 GetInvertedYProjectionMatrix();
    static glm::mat4 GetProjectionMatrix();
    static glm::mat4 GetViewInverseMatrix();
    static glm::mat4 GetViewProjectionMatrix();

    static glm::vec3& GetPosition() { return m_Origin; }

    static void MoveForward();
    static void MoveBackward();
    static void MoveRight();
    static void MoveLeft();

    static void OnRightPressed();
    static void OnMouseMoved(glm::vec2 mousePos);

    static void SetFOV(float fov);
    static float GetFOV();

	static void OnImGui();
    static void Update();

    static void SetAspectRatio(float width, float height);
private:
    inline static glm::vec2 m_DragStartPos = { 0.f, 0.f };

    inline static float m_Fov = 100.f * MathConstants::TO_HALFRADIANS;
    inline static float m_Width = 16.f;
    inline static float m_Height = 9.f;

    inline static  glm::vec3 m_Origin = { 0.f, -2.f, 0.0f };
    inline static glm::vec3 m_Target = { 0.f, -2.f, 0.f };
    inline static float m_MovementSmoothness = 0.02f;

    inline static float m_NearPlane = 0.1f;
    inline static float m_FarPlane = 1000.f;

    inline static glm::vec3 m_Forward = MathConstants::FORWARD;
    inline static glm::vec3 m_Right = MathConstants::RIGHT;
    inline static glm::vec3 m_Up = MathConstants::UP;

    inline static float m_KeyboardMovementSpeed = 0.1f;
    inline static float m_AngularMovementSpeed = 3.0f;
};