#include "stdafx.h"
#include "Mesh.h"
//=============================================================================
Mesh::Mesh(const Vertices& vertices)
{
	SetVertices(vertices);
}
//=============================================================================
Mesh::Mesh(const Vertices& vertices, const Indices& indices)
{
	SetVertices(vertices);
	SetIndices(indices);
}
//=============================================================================
void Mesh::SetVertices(const Vertex* memory, uint32_t count)
{
	m_vertexCount = count;

	if (count == 0) return;

	size_t size = count * sizeof(Vertex);
	size_t stride = sizeof(Vertex);

	if (!m_vertexBuffer.has_value() || m_vertexBuffer.value().GetSize() < size)
		m_vertexBuffer.emplace(size, stride);

	m_vertexBuffer.value().Write(memory, count);
}
//=============================================================================
void Mesh::SetVertices(const Vertices& value)
{
	SetVertices(value.data(), static_cast<uint32_t>(value.size()));
}
//=============================================================================
void Mesh::SetIndices(const Index* memory, uint32_t count)
{
	m_indexCount = count;

	if (count == 0) return;

	size_t size = count * sizeof(Index);
	size_t stride = sizeof(Index);

	if (!m_indexBuffer.has_value() || m_indexBuffer.value().GetSize() < size)
		m_indexBuffer.emplace(size, stride);

	m_indexBuffer.value().Write(memory, count);
}
//=============================================================================
void Mesh::SetIndices(const Indices& value)
{
	SetIndices(value.data(), static_cast<uint32_t>(value.size()));
}
//=============================================================================