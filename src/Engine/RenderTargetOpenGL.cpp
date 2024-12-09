#include "stdafx.h"
#if RENDER_OPENGL
#include "RenderTargetOpenGL.h"
//=============================================================================
RenderTargetGL::RenderTargetGL(TextureGL* texture) : m_texture(texture)
{
	GLint last_fbo;
	GLint last_rbo;

	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &last_fbo);
	glGetIntegerv(GL_RENDERBUFFER_BINDING, &last_rbo);

	glGenRenderbuffers(1, &m_depthStencilRenderbuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, m_depthStencilRenderbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_texture->GetWidth(), m_texture->GetHeight());

	glGenFramebuffers(1, &m_framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_depthStencilRenderbuffer);

#if SE_GFX_VALIDATION_ENABLED
	auto status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	assert(status == GL_FRAMEBUFFER_COMPLETE);
#endif

	glBindFramebuffer(GL_FRAMEBUFFER, last_fbo);
	glBindRenderbuffer(GL_RENDERBUFFER, last_rbo);
}
//=============================================================================
RenderTargetGL::~RenderTargetGL()
{
	glDeleteFramebuffers(1, &m_framebuffer);
	glDeleteRenderbuffers(1, &m_depthStencilRenderbuffer);
}
//=============================================================================
#endif // RENDER_OPENGL