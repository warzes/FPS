#pragma once

#include "Font.h"
#include "RenderUtils.h"

namespace Graphics
{
	struct TextMesh
	{
		enum class Align
		{
			Left,
			Center,
			Right
		};

		Topology topology = Topology::TriangleList;
		utils::Mesh::Vertices vertices;
		utils::Mesh::Indices indices;

		std::vector<glm::vec2> symbol_positions;
		std::vector<glm::vec2> symbol_sizes;
		std::vector<float> symbol_line_y;

		void setSymbolColor(size_t index, const glm::vec4& color);

		static TextMesh createTextMesh(const Font& font, std::wstring::const_iterator begin,
			std::wstring::const_iterator end);

		static TextMesh createSinglelineTextMesh(const Font& font, const std::wstring& text,
			float vertical_offset = 0.0f);

		static std::tuple<float, TextMesh> createMultilineTextMesh(const Font& font, const std::wstring& text,
			float maxWidth, float size, Align align = Align::Left);
	};
}
