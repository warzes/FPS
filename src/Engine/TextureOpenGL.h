#pragma once

#if RENDER_OPENGL

#include "RenderCoreOpenGL.h"

class TextureGL
{
public:
	class ScopedBind
	{
	public:
		ScopedBind(GLuint texture)
		{
			glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_lastTexture);
			glBindTexture(GL_TEXTURE_2D, texture);
		}

		~ScopedBind()
		{
			glBindTexture(GL_TEXTURE_2D, m_lastTexture);
		}

	private:
		GLint m_lastTexture = 0;
	};

	TextureGL(uint32_t width, uint32_t height, PixelFormat format, uint32_t mip_count);
	~TextureGL();

	void Write(uint32_t width, uint32_t height, PixelFormat format, const void* memory, uint32_t mip_level, uint32_t offset_x, uint32_t offset_y);

	void GenerateMips();

	auto GetGLTexture() const { return m_texture; }
	auto GetWidth() const { return m_width; }
	auto GetHeight() const { return m_height; }
	auto GetFormat() const { return m_format; }
	auto GetMipCount() const { return m_mipCount; }

private:
	GLuint m_texture = 0;
	uint32_t m_width = 0;
	uint32_t m_height = 0;
	uint32_t m_mipCount = 0;
	PixelFormat m_format;
};

#endif // RENDER_OPENGL