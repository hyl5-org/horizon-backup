#include "Camera.h"

namespace Horizon {
Camera::Camera(Math::float3 eye, Math::float3 at, Math::float3 up) noexcept : m_eye(eye), m_at(at), m_up(up) {
    m_forward = Math::Normalize(m_at - m_eye);
    m_right = Math::Cross(m_forward, m_up);
    UpdateViewMatrix();
    // setLookAt(eye, at, up);
}

void Camera::SetPerspectiveProjectionMatrix(f32 fov, f32 aspect_ratio, f32 nearPlane, f32 farPlane) noexcept {
    m_fov = fov;
    m_aspect_ratio = aspect_ratio;
    m_near_plane = nearPlane;
    m_far_plane = farPlane;
    // m_projection = Math::perspective(fov, aspect_ratio, nearPlane, farPlane);
    // m_projection = ReversePerspective(fov, aspect_ratio, nearPlane,
    // farPlane);
}

Math::float4x4 Camera::GetProjectionMatrix() const noexcept { return m_projection; }

Math::float3 Camera::GetFov() const noexcept { return Math::float3(); }

Math::float2 Camera::GetNearFarPlane() const noexcept { return Math::float2(m_near_plane, m_far_plane); }

void Camera::SetCameraSpeed(f32 speed) noexcept { m_camera_speed = speed; }

f32 Camera::GetCameraSpeed() const noexcept { return m_camera_speed; }

void Camera::Move(Direction direction) noexcept {
    switch (direction) {
    case Horizon::Direction::FORWARD:
        m_eye += GetCameraSpeed() * m_forward;
        break;
    case Horizon::Direction::BACKWARD:
        m_eye -= GetCameraSpeed() * m_forward;
        break;
    case Horizon::Direction::RIGHT:
        m_eye += GetCameraSpeed() * m_right;
        break;
    case Horizon::Direction::LEFT:
        m_eye -= GetCameraSpeed() * m_right;
        break;
    case Horizon::Direction::UP:
        m_eye += GetCameraSpeed() * m_up;
        break;
    case Horizon::Direction::DOWN:
        m_eye -= GetCameraSpeed() * m_up;
        break;
    default:
        break;
    }
}
void Camera::Rotate(f32 xoffset, f32 yoffset) noexcept {
    m_yaw += xoffset;
    m_pitch -= yoffset; // TODO: unify axis in different API

    // prevent locked
    if (m_pitch > 89.0f)
        m_pitch = 89.0f;
    if (m_pitch < -89.0f)
        m_pitch = -89.0f;
}
void Camera::UpdateViewMatrix() noexcept {
    // calculate the new Front std::vector
    Math::float3 front;
    front.x = cos(Math::Radians(m_yaw)) * cos(Math::Radians(m_pitch));
    front.y = sin(Math::Radians(m_pitch));
    front.z = sin(Math::Radians(m_yaw)) * cos(Math::Radians(m_pitch));

    m_forward = Math::Normalize(front);
    m_right = Math::Normalize(Math::Cross(m_forward, Math::float3(0.0, 1.0,
                                                                  0.0))); // normalize the vectors, because their length
                                                                          // gets closer to 0 the more you look up or
                                                                          // down which results in slower Movement.
    m_up = Math::Normalize(Math::Cross(m_right, m_forward));

    m_view = Math::LookAt(m_eye, m_eye + m_forward, m_up);
}
Math::float4x4 Camera::GetInvViewProjectionMatrix() const noexcept {
    // TODO: the directx matrix calculation rule
    return (m_projection * m_view).Invert();
}
Math::float3 Camera::GetForwardDir() const noexcept { return m_forward; }
Math::float4x4 Camera::GetViewMatrix() const noexcept { return m_view; }
Math::float3 Camera::GetPosition() const noexcept { return m_eye; }
} // namespace Horizon
