#include "stdafx.h"
#if RENDER_OPENGL
#include "TextureOpenGL.h"
//=============================================================================
TextureGL::TextureGL(uint32_t width, uint32_t height, PixelFormat format, uint32_t mip_count) :
	m_width(width),
	m_height(height),
	m_format(format),
	m_mipCount(mip_count)
{
	glGenTextures(1, &m_texture);

	auto internal_format = TextureInternalFormatMap.at(m_format);
	auto texture_format = TextureFormatMap.at(m_format);
	auto format_type = PixelFormatTypeMap.at(m_format);
	auto binding = ScopedBind(m_texture);

	for (uint32_t i = 0; i < mip_count; i++)
	{
		auto mip_width = GetMipWidth(width, i);
		auto mip_height = GetMipHeight(height, i);
		glTexImage2D(GL_TEXTURE_2D, i, internal_format, mip_width, mip_height, 0, texture_format, format_type, NULL);
	}
}
//=============================================================================
TextureGL::~TextureGL()
{
	glDeleteTextures(1, &m_texture);
}
//=============================================================================
void TextureGL::Write(uint32_t width, uint32_t height, PixelFormat format, const void* memory, uint32_t mip_level, uint32_t offset_x, uint32_t offset_y)
{
	auto channels_count = GetFormatChannelsCount(format);
	auto channel_size = GetFormatChannelSize(format);
	auto format_type = PixelFormatTypeMap.at(format);
	auto texture_format = TextureFormatMap.at(format);

	auto row_size = width * channels_count * channel_size;
	auto image_size = height * row_size;
	auto flipped_image = std::vector<uint8_t>(image_size);

	for (uint32_t i = 0; i < height; i++)
	{
		auto src = (void*)(size_t(memory) + (size_t(i) * row_size));
		auto dst = (void*)(size_t(flipped_image.data()) + (size_t(height - 1 - i) * row_size));
		memcpy(dst, src, row_size);
	}

	auto mip_height = GetMipHeight(m_height, mip_level);
	auto binding = ScopedBind(m_texture);

	glTexSubImage2D(GL_TEXTURE_2D, mip_level, offset_x, (mip_height - height) - offset_y, width, height,
		texture_format, format_type, flipped_image.data());
}
//=============================================================================
void TextureGL::GenerateMips()
{
	auto binding = ScopedBind(m_texture);
	glGenerateMipmap(GL_TEXTURE_2D);
}
//=============================================================================
#endif // RENDER_OPENGL