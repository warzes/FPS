#include "stdafx.h"
#include "Camera.h"
//=============================================================================
glm::mat4 Camera2D::GetViewMatrix() const
{
	auto eye = glm::vec3(0.0f, 0.0f, 0.0f);
	auto center = glm::vec3(0.0f, 0.0f, 1.0f);
	auto up = glm::vec3(0.0f, 1.0f, 0.0f);

	auto view = glm::lookAtLH(eye, center, up);

	view = glm::translate(view, glm::vec3({ m_width / 2.0f, m_height / 2.0f, 0.0f }));
	view = glm::translate(view, { -m_position, 0.0f });

	return view;
}
//=============================================================================
glm::mat4 Camera2D::GetProjectionMatrix() const
{
	return glm::orthoLH(0.0f, (float)m_width, (float)m_height, 0.0f, -1.0f, 1.0f);
}
//=============================================================================
void Camera2D::SetSize(uint32_t width, uint32_t height)
{
	m_width = width;
	m_height = height;
}
//=============================================================================
void Camera3D::SetSize(uint32_t width, uint32_t height)
{
	m_width = width;
	m_height = height;
}
//=============================================================================
void Camera3D::Update()
{
	auto yaw = m_yaw;// *(mWorldUp.y * -1.0f);
	auto pitch = m_pitch;// *(mWorldUp.y * -1.0f);

	float sinYaw = glm::sin(yaw);
	float sinPitch = glm::sin(pitch);

	float cosYaw = glm::cos(yaw);
	float cosPitch = glm::cos(pitch);

	m_front = glm::normalize(glm::vec3(cosYaw * cosPitch, sinPitch, sinYaw * cosPitch));
	m_right = glm::normalize(glm::cross(m_front, m_worldUp));
	m_up = glm::normalize(glm::cross(m_right, m_front));

	auto width = static_cast<float>(m_width);
	auto height = static_cast<float>(m_height);

	m_viewMatrix = glm::lookAtRH(m_position, m_position + m_front, m_up);
	m_projectionMatrix = glm::perspectiveFov(m_fieldOfView, width, height, m_nearPlane, m_farPlane);
}
//=============================================================================
void Camera3D::FrontMove(float offset)
{
	m_position += m_front * offset;
}
//=============================================================================
void Camera3D::SideMove(float offset)
{
	m_position += m_right * offset;
}
//=============================================================================