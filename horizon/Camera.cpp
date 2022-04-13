#include "Camera.h"

namespace Horizon
{
	Camera::Camera(Math::vec3 eye, Math::vec3 at, Math::vec3 up) :m_eye(eye), m_at(at), m_up(up)
	{
		m_forward = normalize(m_at - m_eye);
		m_right = cross(m_forward, m_up);
		updateViewMatrix();
		//setLookAt(eye, at, up);
	}
	void Camera::setPerspectiveProjectionMatrix(f32 fov, float aspect_ratio, float nearPlane, float farPlane)
	{
		m_fov = fov;
		m_aspect_ratio = aspect_ratio;
		m_near_plane = nearPlane;
		m_far_plane = farPlane;
		m_projection = Math::perspective(fov, aspect_ratio, nearPlane, farPlane);
	}
	Math::mat4 Camera::getProjectionMatrix() const
	{
		return m_projection;
	}

	Math::vec3 Camera::getFov() const
	{
		return Math::vec3();
	}
	Math::vec2 Camera::getNearFarPlane() const
	{
		return Math::vec2(m_near_plane, m_far_plane);
	}
	void Camera::setCameraSpeed(f32 speed)
	{
		m_camera_speed = speed;
	}
	f32 Camera::getCameraSpeed() const
	{
		return m_camera_speed;
	}

	void Camera::move(Direction direction)
	{
		switch (direction)
		{
		case Horizon::Direction::FORWARD:
			m_eye += m_camera_speed * m_forward;
			break;
		case Horizon::Direction::BACKWARD:
			m_eye -= m_camera_speed * m_forward;
			break;
		case Horizon::Direction::RIGHT:
			m_eye += m_camera_speed * m_right;
			break;
		case Horizon::Direction::LEFT:
			m_eye -= m_camera_speed * m_right;
			break;
		case Horizon::Direction::UP:
			m_eye += m_camera_speed * m_up;
			break;
		case Horizon::Direction::DOWN:
			m_eye -= m_camera_speed * m_up;
			break;
		default:
			break;
		}
	}
	void Camera::rotate(f32 xoffset, f32 yoffset)
	{
		m_yaw += xoffset;
		m_pitch -= yoffset; // TODO: unify axis in different API

		// prevent locked
		if (m_pitch > 89.0f)
			m_pitch = 89.0f;
		if (m_pitch < -89.0f)
			m_pitch = -89.0f;
	}
	void Camera::updateViewMatrix()
	{
		// calculate the new Front std::vector
		Math::vec3 front;
		front.x = cos(Math::radians(m_yaw)) * cos(Math::radians(m_pitch));
		front.y = sin(Math::radians(m_pitch));
		front.z = sin(Math::radians(m_yaw)) * cos(Math::radians(m_pitch));

		m_forward = Math::normalize(front);
		m_right = Math::normalize(Math::cross(m_forward, Math::vec3(0.0, 1.0, 0.0)));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
		m_up = Math::normalize(Math::cross(m_right, m_forward));

		m_view = Math::lookAt(m_eye, m_eye + m_forward, m_up);
	}
	Math::mat4 Camera::getViewMatrix() const
	{
		return m_view;
	}
	Math::vec3 Camera::getPosition() const
	{
		return m_eye;
	}
} // namespace Horizon
