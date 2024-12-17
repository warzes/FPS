#pragma once

#include "Mesh.h"

namespace utils
{
	class MeshBuilder final
	{
	public:
		enum class Mode
		{
			Points,
			Lines,
			LineLoop,
			LineStrip,
			Triangles,
			TriangleStrip,
			TriangleFan,
			Quads,
			Polygon
		};

		void Reset();
		void Begin(Mode mode, std::function<void()> onFlush = nullptr);
		void Vertex(const Mesh::Vertex& value);
		void End();
		void SetToMesh(Mesh& mesh);

		bool IsBegan() const { return m_began; }
		const auto& GetTopology() const { return m_topology; }
		const auto& GetVertices() const { return m_vertices; }
		const auto& GetIndices() const { return m_indices; }

	private:
		bool                    m_began = false;
		std::optional<Mode>     m_mode;
		std::optional<Topology> m_topology;
		Mesh::Vertices          m_vertices;
		Mesh::Indices           m_indices;
		uint32_t                m_vertexStart = 0;
	};
} // namespace utils