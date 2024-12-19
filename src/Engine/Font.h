#pragma once

#include "RHIResources.h"

class Font final
{
public:
	static inline const float GlyphSize = 32.0f;
	static inline const float SdfPadding = GlyphSize / 8.0f;
	static inline const float SdfOnedge = 0.5f;

	struct Glyph final
	{
		glm::vec2 pos;
		glm::vec2 size;
		glm::vec2 offset;
		float     xAdvance;
	};

	Font(void* data, size_t size);

	auto GetTexture() const { return m_texture; }
	const Glyph& GetGlyph(wchar_t symbol) const;

	static float GetScaleFactorForSize(float size);

	float GetStringWidth(std::wstring::const_iterator begin, std::wstring::const_iterator end, float size = GlyphSize) const;
	float GetStringWidth(const std::wstring& text, float size = GlyphSize) const;

	float GetKerning(wchar_t left, wchar_t right) const;

	float GetAscent() const { return m_ascent; }
	float GetDescent() const { return m_descent; }
	float GetLinegap() const { return m_linegap; }

	float GetCustomVerticalOffset() const { return m_customVerticalOffset; }
	void SetCustomVerticalOffset(float value) { m_customVerticalOffset = value; }

private:
	std::shared_ptr<Texture2D>                                        m_texture{ nullptr };
	std::unordered_map<wchar_t, Glyph>                              m_glyphs;
	std::unordered_map<wchar_t, std::unordered_map<wchar_t, float>> m_kernings;
	float                                                           m_ascent = 0.0f;
	float                                                           m_descent = 0.0f;
	float                                                           m_linegap = 0.0f;
	float                                                           m_customVerticalOffset = 0.0f; // TODO: should be removed, code must work without this
};