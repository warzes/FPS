#pragma once

#include "Vertex.h"
#include "RenderResources.h"

class Mesh
{
public:
	using Vertex = vertex::PositionColorTextureNormalTangent;
	using Index = uint32_t;
	using Vertices = std::vector<Vertex>;
	using Indices = std::vector<Index>;

public:
	Mesh() = default;
	Mesh(const Vertices& vertices);
	Mesh(const Vertices& vertices, const Indices& indices);

	void SetVertices(const Vertex* memory, uint32_t count);
	void SetVertices(const Vertices& value);

	void SetIndices(const Index* memory, uint32_t count);
	void SetIndices(const Indices& value);

	auto GetVertexCount() const { return m_vertexCount; }
	auto GetIndexCount() const { return m_indexCount; }

	const auto& GetVertexBuffer() const { return m_vertexBuffer; }
	const auto& GetIndexBuffer() const { return m_indexBuffer; }

private:
	uint32_t m_vertexCount = 0;
	uint32_t m_indexCount = 0;
	std::optional<VertexBuffer> m_vertexBuffer;
	std::optional<IndexBuffer> m_indexBuffer;
};

struct DirectionalLight
{
	glm::vec3 direction = { 0.5f, 0.5f, 0.5f };
	glm::vec3 ambient = { 1.0f, 1.0f, 1.0f };
	glm::vec3 diffuse = { 1.0f, 1.0f, 1.0f };
	glm::vec3 specular = { 1.0f, 1.0f, 1.0f };
	float shininess = 32.0f;
};

struct PointLight
{
	glm::vec3 position = { 0.0f, 0.0f, 0.0f };
	glm::vec3 ambient = { 1.0f, 1.0f, 1.0f };
	glm::vec3 diffuse = { 1.0f, 1.0f, 1.0f };
	glm::vec3 specular = { 1.0f, 1.0f, 1.0f };
	float constant_attenuation = 0.0f;
	float linear_attenuation = 0.00128f;
	float quadratic_attenuation = 0.0f;
	float shininess = 32.0f;
};

using Light = std::variant<
	DirectionalLight,
	PointLight
>;

struct OrthogonalCamera
{
	std::optional<uint32_t> width = std::nullopt;
	std::optional<uint32_t> height = std::nullopt;
};

struct PerspectiveCamera
{
	std::optional<uint32_t> width = std::nullopt;
	std::optional<uint32_t> height = std::nullopt;

	float yaw = 0.0f;
	float pitch = 0.0f;
	glm::vec3 position = { 0.0f, 0.0f, 0.0f };
	glm::vec3 world_up = { 0.0f, 1.0f, 0.0f };
	float far_plane = 8192.0f;
	float near_plane = 1.0f;
	float fov = 70.0f;
};

using Camera = std::variant<OrthogonalCamera, PerspectiveCamera>;

std::tuple<glm::mat4/*proj*/, glm::mat4/*view*/> MakeCameraMatrices(const OrthogonalCamera& camera);
std::tuple<glm::mat4/*proj*/, glm::mat4/*view*/> MakeCameraMatrices(const PerspectiveCamera& camera);

inline std::tuple<glm::mat4/*view*/, glm::mat4/*projection*/> CalculatePerspectiveViewProjection(float yaw,
	float pitch, const glm::vec3& position, uint32_t width, uint32_t height, float fov = 70.0f,
	float near_plane = 1.0f, float far_plane = 8192.0f, const glm::vec3& world_up = { 0.0f, 1.0f, 0.0f })
{
	auto [proj, view] = MakeCameraMatrices(PerspectiveCamera{
		.width = width,
		.height = height,
		.yaw = yaw,
		.pitch = pitch,
		.position = position,
		.world_up = world_up,
		.far_plane = far_plane,
		.near_plane = near_plane,
		.fov = fov
		});
	return { view, proj };
}

inline std::tuple<uint32_t, uint32_t, void*> LoadTexture(const std::string& filename)
{
	int width = 0;
	int height = 0;
	void* memory = stbi_load(filename.c_str(), &width, &height, nullptr, 4);
	return { width, height, memory };
}