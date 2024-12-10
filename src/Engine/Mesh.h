#pragma once

#include "Vertex.h"
#include "RHIResources.h"

class Mesh final
{
public:
	using Vertex   = vertex::PositionColorTextureNormalTangent;
	using Index    = uint32_t;
	using Vertices = std::vector<Vertex>;
	using Indices  = std::vector<Index>;

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
	uint32_t                    m_vertexCount = 0;
	uint32_t                    m_indexCount = 0;
	std::optional<VertexBuffer> m_vertexBuffer;
	std::optional<IndexBuffer>  m_indexBuffer;
};