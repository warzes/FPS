#pragma once

#if RENDER_OPENGL

#include "TextureOpenGL.h"

class RenderTargetGL
{
public:
	RenderTargetGL(TextureGL* texture);
	~RenderTargetGL();

	auto GetGLFramebuffer() const { return m_framebuffer; }
	auto GetTexture() const { return m_texture; }

private:
	GLuint m_framebuffer = 0;
	GLuint m_depthStencilRenderbuffer = 0;
	TextureGL* m_texture = nullptr;
};

#endif // RENDER_OPENGL