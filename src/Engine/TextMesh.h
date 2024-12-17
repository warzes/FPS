#pragma once

#include "Font.h"
#include "RenderUtils.h"

class TextMesh final
{
public:
	enum class Align
	{
		Left,
		Center,
		Right
	};

	void SetSymbolColor(size_t index, const glm::vec4& color);

	static TextMesh CreateTextMesh(const Font& font, std::wstring::const_iterator begin, std::wstring::const_iterator end);
	static TextMesh CreateSingleLineTextMesh(const Font& font, const std::wstring& text, float vertical_offset = 0.0f);
	static std::tuple<float, TextMesh> CreateMultilineTextMesh(const Font& font, const std::wstring& text, float maxWidth, float size, Align align = Align::Left);

	Topology               topology{ Topology::TriangleList };
	Mesh::Vertices         vertices;
	Mesh::Indices          indices;

	std::vector<glm::vec2> symbolPositions;
	std::vector<glm::vec2> symbolSizes;
	std::vector<float>     symbolLineY;
};