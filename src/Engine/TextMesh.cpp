#include "stdafx.h"
#include "TextMesh.h"
//=============================================================================
TextMesh TextMesh::CreateTextMesh(const Font& font, std::wstring::const_iterator begin, std::wstring::const_iterator end)
{
	TextMesh mesh;

	mesh.topology = Topology::TriangleList;

	const auto texture = font.GetTexture();

	glm::vec2 tex_size = { static_cast<float>(texture->GetWidth()), static_cast<float>(texture->GetHeight()) };

	auto length = std::distance(begin, end);

	mesh.vertices.resize(length * 4);
	mesh.indices.resize(length * 6);
	mesh.symbolPositions.resize(length);
	mesh.symbolSizes.resize(length);
	mesh.symbolLineY.resize(length);

	glm::vec2 pos = { 0.0f, 0.0f };

	int i = 0;

	for (auto it = begin; it != end; ++it, i++)
	{
		const auto& glyph = font.GetGlyph(*it);

		auto vtx = &mesh.vertices[i * 4];
		auto idx = &mesh.indices[i * 6];

		pos.x += glyph.offset.x;
		pos.y = font.GetAscent() + glyph.offset.y;

		auto p1 = pos;
		auto p2 = pos + glyph.size;

		pos.x -= glyph.offset.x;
		pos.x += glyph.xAdvance;

		if (it != end - 1)
		{
			pos.x += font.GetKerning(*it, *(it + 1));
		}

		auto uv1 = glyph.pos / tex_size;
		auto uv2 = (glyph.pos + glyph.size) / tex_size;

		glm::vec4 color = glm::vec4{ 1.0f };

		vtx[0] = { { p1.x, p1.y, 0.0f }, color, { uv1.x, uv1.y } };
		vtx[1] = { { p1.x, p2.y, 0.0f }, color, { uv1.x, uv2.y } };
		vtx[2] = { { p2.x, p2.y, 0.0f }, color, { uv2.x, uv2.y } };
		vtx[3] = { { p2.x, p1.y, 0.0f }, color, { uv2.x, uv1.y } };

		auto base_vtx = i * 4;

		idx[0] = base_vtx + 0;
		idx[1] = base_vtx + 1;
		idx[2] = base_vtx + 2;
		idx[3] = base_vtx + 0;
		idx[4] = base_vtx + 2;
		idx[5] = base_vtx + 3;

		mesh.symbolPositions[i] = p1;
		mesh.symbolSizes[i] = glyph.size;
		mesh.symbolLineY[i] = 0.0f;
	}

	return mesh;
}
//=============================================================================
TextMesh TextMesh::CreateSingleLineTextMesh(const Font& font, const std::wstring& text, float vertical_offset)
{
	auto mesh = CreateTextMesh(font, text.begin(), text.end());

	for (auto& vertex : mesh.vertices)
	{
		vertex.pos.y += vertical_offset;
	}

	return mesh;
}
//=============================================================================
std::tuple<float, TextMesh> TextMesh::CreateMultilineTextMesh(const Font& font, const std::wstring& text, float maxWidth, float size, Align align)
{
	auto scale = font.GetScaleFactorForSize(size);
	float height = 0.0f;

	float scaledMaxWidth = maxWidth / scale;

	TextMesh result;

	auto appendTextMesh = [&font, scaledMaxWidth, &height, &result, align](std::wstring::const_iterator begin, std::wstring::const_iterator end) {
		auto mesh = CreateTextMesh(font, begin, end);
		for (auto index : mesh.indices)
		{
			index += static_cast<uint32_t>(result.vertices.size());
			result.indices.push_back(index);
		}
		auto str_w = font.GetStringWidth(begin, end);
		for (auto vertex : mesh.vertices)
		{
			if (align == Align::Right)
				vertex.pos.x += scaledMaxWidth - str_w;
			else if (align == Align::Center)
				vertex.pos.x += (scaledMaxWidth - str_w) / 2.0f;

			vertex.pos.y += height;
			result.vertices.push_back(vertex);
		}

		for (auto symbol_position : mesh.symbolPositions)
		{
			if (align == Align::Right)
				symbol_position.x += scaledMaxWidth - str_w;
			else if (align == Align::Center)
				symbol_position.x += (scaledMaxWidth - str_w) / 2.0f;

			symbol_position.y += height;
			result.symbolPositions.push_back(symbol_position);
		}

		for (auto symbol_size : mesh.symbolSizes)
		{
			result.symbolSizes.push_back(symbol_size);
		}

		for (auto symbol_line_y : mesh.symbolLineY)
		{
			symbol_line_y += height;
			result.symbolLineY.push_back(symbol_line_y);
		}

		height += font.GetAscent() - font.GetDescent() + font.GetLinegap();
		};

	result.topology = Topology::TriangleList;

	auto begin = text.begin();
	auto it = begin;

	while (it != text.end())
	{
		std::advance(it, 1);

		if (it != text.end() && *it == '\n')
		{
			appendTextMesh(begin, it);
			begin = it + 1;
			continue;
		}

		auto length = std::distance(begin, it);

		if (length <= 1)
			continue;

		auto str_w = font.GetStringWidth(begin, it);

		if (str_w <= scaledMaxWidth)
			continue;

		--it;

		auto best_it = it;

		while (best_it != begin)
		{
			if (*best_it == ' ')
			{
				++best_it;
				break;
			}

			--best_it;
		}

		if (best_it != begin)
			it = best_it;

		appendTextMesh(begin, it);
		begin = it;
	}

	if (begin != text.end())
		appendTextMesh(begin, text.end());

	height -= font.GetLinegap();

	return { height * scale, result };
}
//=============================================================================
void TextMesh::SetSymbolColor(size_t index, const glm::vec4& color)
{
	size_t base_vtx = index * 4;

	for (size_t i = base_vtx; i < base_vtx + 4; i++)
	{
		vertices[i].color = color;
	}
}
//=============================================================================