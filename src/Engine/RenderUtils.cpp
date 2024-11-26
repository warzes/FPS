#include "stdafx.h"
#include "RenderUtils.h"

Mesh::Mesh(const Vertices& vertices)
{
	SetVertices(vertices);
}

Mesh::Mesh(const Vertices& vertices, const Indices& indices)
{
	SetVertices(vertices);
	SetIndices(indices);
}

void Mesh::SetVertices(const Vertex* memory, uint32_t count)
{
	m_vertexCount = count;

	if (count == 0)
		return;

	size_t size = count * sizeof(Vertex);
	size_t stride = sizeof(Vertex);

	if (!m_vertexBuffer.has_value() || m_vertexBuffer.value().GetSize() < size)
		m_vertexBuffer.emplace(size, stride);

	m_vertexBuffer.value().Write(memory, count);
}

void Mesh::SetVertices(const Vertices& value)
{
	SetVertices(value.data(), static_cast<uint32_t>(value.size()));
}

void Mesh::SetIndices(const Index* memory, uint32_t count)
{
	m_indexCount = count;

	if (count == 0)
		return;

	size_t size = count * sizeof(Index);
	size_t stride = sizeof(Index);

	if (!m_indexBuffer.has_value() || m_indexBuffer.value().GetSize() < size)
		m_indexBuffer.emplace(size, stride);

	m_indexBuffer.value().Write(memory, count);
}

void Mesh::SetIndices(const Indices& value)
{
	SetIndices(value.data(), static_cast<uint32_t>(value.size()));
}

std::tuple<glm::mat4/*proj*/, glm::mat4/*view*/> MakeCameraMatrices(const OrthogonalCamera& camera)
{
	float width = (float)camera.width.value_or(1);
	float height = (float)camera.height.value_or(1);
	auto proj = glm::orthoLH(0.0f, width, height, 0.0f, -1.0f, 1.0f);
	auto view = glm::lookAtLH(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f),
		glm::vec3(0.0f, 1.0f, 0.0f));
	return { proj, view };
}

std::tuple<glm::mat4/*proj*/, glm::mat4/*view*/> MakeCameraMatrices(const PerspectiveCamera& camera)
{
	auto sin_yaw = glm::sin(camera.yaw);
	auto sin_pitch = glm::sin(camera.pitch);

	auto cos_yaw = glm::cos(camera.yaw);
	auto cos_pitch = glm::cos(camera.pitch);

	auto front = glm::normalize(glm::vec3(cos_yaw * cos_pitch, sin_pitch, sin_yaw * cos_pitch));
	auto right = glm::normalize(glm::cross(front, camera.world_up));
	auto up = glm::normalize(glm::cross(right, front));

	auto width = (float)camera.width.value_or(1);
	auto height = (float)camera.height.value_or(1);

	auto proj = glm::perspectiveFov(camera.fov, width, height, camera.near_plane, camera.far_plane);
	auto view = glm::lookAtRH(camera.position, camera.position + front, up);

	return { proj, view };
}