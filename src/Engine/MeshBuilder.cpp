#include "stdafx.h"
#include "MeshBuilder.h"
#include "Log.h"
//=============================================================================
void addIndicesForVertexArray(utils::MeshBuilder::Mode mode, Topology topology, uint32_t vertex_start, uint32_t vertex_count, std::vector<uint32_t>& indices)
{
	auto extract_ordered_index_sequence = [](uint32_t vertex_start, uint32_t vertex_count, auto& indices) {
		indices.reserve(indices.size() + (size_t)(vertex_count - vertex_start));

		for (uint32_t i = vertex_start; i < vertex_count; i++)
			indices.push_back(i);
		};

	auto add_indices = [](auto& indices, auto... elements) {
		(indices.push_back(elements), ...);
		};

	auto extract_line_list_indices_from_line_strip = [&add_indices](uint32_t vertex_start, uint32_t vertex_count, auto& indices) {
		for (uint32_t i = vertex_start + 1; i < vertex_count; i++)
			add_indices(indices, i - 1, i);
		};

	auto extract_line_list_indices_from_line_loop = [&extract_line_list_indices_from_line_strip, &add_indices](uint32_t vertex_start, uint32_t vertex_count, auto& indices) {
		if (vertex_count == vertex_start)
			return;

		extract_line_list_indices_from_line_strip(vertex_start, vertex_count, indices);
		add_indices(indices, vertex_start + vertex_count - 1, vertex_start);
		};

	auto extract_triangles_indices_from_triangle_fan = [&add_indices](uint32_t vertex_start, uint32_t vertex_count, auto& indices) {
		for (uint32_t i = vertex_start + 2; i < vertex_count; i++)
			add_indices(indices, vertex_start, i - 1, i);
		};

	auto extract_triangles_indices_from_polygons = extract_triangles_indices_from_triangle_fan;

	auto extract_triangles_indices_from_quads = [&add_indices](uint32_t vertex_start, uint32_t vertex_count, auto& indices) {
		for (uint32_t i = vertex_start + 3; i < vertex_count; i += 4)
			add_indices(indices, i - 3, i - 2, i - 1, i - 3, i - 1, i);
		};

	auto extract_triangles_indices_from_triangle_strip = [&add_indices](uint32_t vertex_start, uint32_t vertex_count, auto& indices) {
		for (uint32_t i = vertex_start + 2; i < vertex_count; i++)
		{
			bool even = (i - vertex_start) % 2 == 0;
			add_indices(indices, i - (even ? 2 : 1), i - (even ? 1 : 2), i);
		}
		};

	using ExtractIndicesFunc = std::function<void(uint32_t vertex_start,
		uint32_t vertex_count, std::vector<uint32_t>& indices)>;

	static const std::unordered_map<utils::MeshBuilder::Mode, std::unordered_map<Topology, ExtractIndicesFunc>> ExtractIndicesFuncs = {
		{ utils::MeshBuilder::Mode::Points, { { Topology::PointList, extract_ordered_index_sequence } } },
		{ utils::MeshBuilder::Mode::Lines, { { Topology::LineList, extract_ordered_index_sequence } } },
		{ utils::MeshBuilder::Mode::LineLoop, { { Topology::LineList, extract_line_list_indices_from_line_loop } } },
		{ utils::MeshBuilder::Mode::LineStrip, { { Topology::LineList, extract_line_list_indices_from_line_strip } } },
		{ utils::MeshBuilder::Mode::Polygon, { { Topology::TriangleList, extract_triangles_indices_from_polygons } } },
		{ utils::MeshBuilder::Mode::TriangleFan, { { Topology::TriangleList, extract_triangles_indices_from_triangle_fan } } },
		{ utils::MeshBuilder::Mode::Quads, { { Topology::TriangleList, extract_triangles_indices_from_quads } } },
		{ utils::MeshBuilder::Mode::TriangleStrip, { { Topology::TriangleList, extract_triangles_indices_from_triangle_strip } } },
		{ utils::MeshBuilder::Mode::Triangles, { { Topology::TriangleList, extract_ordered_index_sequence } } }
	};

	ExtractIndicesFuncs.at(mode).at(topology)(vertex_start, vertex_count, indices);
}
//=============================================================================
void utils::MeshBuilder::Reset()
{
	if (m_began)
		Fatal("missing end()");

	m_indices.clear();
	m_vertices.clear();
	m_mode.reset();
	m_topology.reset();
}
//=============================================================================
void utils::MeshBuilder::Begin(Mode mode, std::function<void()> onFlush)
{
	if (m_began)
		Fatal("missing end()");

	m_began = true;

	static const std::unordered_map<Mode, Topology> TopologyMap = {
		{ Mode::Points, Topology::PointList },
		{ Mode::Lines, Topology::LineList },
		{ Mode::LineLoop, Topology::LineList },
		{ Mode::LineStrip, Topology::LineList },
		{ Mode::Triangles, Topology::TriangleList },
		{ Mode::TriangleStrip, Topology::TriangleList },
		{ Mode::TriangleFan, Topology::TriangleList },
		{ Mode::Quads, Topology::TriangleList },
		{ Mode::Polygon, Topology::TriangleList }
	};

	auto topology = TopologyMap.at(mode);

	if (m_topology.has_value() && topology != m_topology.value() && onFlush)
		onFlush();

	m_topology = topology;
	m_mode = mode;
	m_vertexStart = (uint32_t)m_vertices.size();
}
//=============================================================================
void utils::MeshBuilder::Vertex(const Mesh::Vertex& value)
{
	if (!m_began)
		Fatal("missing begin()");

	m_vertices.push_back(value);
}
//=============================================================================
void utils::MeshBuilder::End()
{
	if (!m_began)
		Fatal("missing begin()");

	m_began = false;
	addIndicesForVertexArray(m_mode.value(), m_topology.value(), m_vertexStart, (uint32_t)m_vertices.size(), m_indices);
}
//=============================================================================
void utils::MeshBuilder::SetToMesh(Mesh& mesh)
{
	if (m_began)
		Fatal("missing end()");

	mesh.SetVertices(m_vertices);
	mesh.SetIndices(m_indices);
}
//=============================================================================