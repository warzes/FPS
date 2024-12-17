#pragma once

class Camera2D final 
{
public:
	glm::mat4 GetViewMatrix() const;
	glm::mat4 GetProjectionMatrix() const;

	auto GetPosition() const { return m_position; }
	void SetPosition(glm::vec2 value) { m_position = value; }

	void SetSize(uint32_t width, uint32_t height);

private:
	glm::vec2 m_position = { 0.0f, 0.0f };
	uint32_t  m_width{ 0 };
	uint32_t  m_height{ 0 };
};

class Camera3D final
{
public:
	void Update();

	void SetSize(uint32_t width, uint32_t height);

	void FrontMove(float offset);
	void SideMove(float offset);

	auto GetViewMatrix() const { return m_viewMatrix; }
	auto GetProjectionMatrix() const { return m_projectionMatrix; }

	auto GetPosition() const { return m_position; }
	void SetPosition(glm::vec3 value) { m_position = value; }

	auto GetYaw() const { return m_yaw; }
	void SetYaw(float value) { m_yaw = value; }

	auto GetPitch() const { return m_pitch; }
	void SetPitch(float value) { m_pitch = value; }

	auto GetFieldOfView() const { return m_fieldOfView; }
	void SetFieldOfView(float value) { m_fieldOfView = value; }

	auto GetWorldUp() const { return m_worldUp; }
	void SetWorldUp(const glm::vec3& value) { m_worldUp = value; }

	auto GetNearPlane() const { return m_nearPlane; }
	void SetNearPlane(float value) { m_nearPlane = value; }

	auto GetFarPlane() const { return m_farPlane; }
	void SetFarPlane(float value) { m_farPlane = value; }

	auto GetFront() const { return m_front; }
	auto GetUp() const { return m_up; }
	auto GetRight() const { return m_right; }

private:
	glm::mat4 m_viewMatrix = glm::mat4(1.0f);
	glm::mat4 m_projectionMatrix = glm::mat4(1.0f);
	uint32_t  m_width{ 0 };
	uint32_t  m_height{ 0 };

	glm::vec3 m_position = { 0.0f, 0.0f, 0.0f };
	float     m_yaw = 0.0f;
	float     m_pitch = 0.0f;
	float     m_fieldOfView = 70.0f;
	glm::vec3 m_worldUp = { 0.0f, -1.0f, 0.0f };
	float     m_nearPlane = 1.0f;
	float     m_farPlane = 8192.0f;

	glm::vec3 m_front;
	glm::vec3 m_up;
	glm::vec3 m_right;
};