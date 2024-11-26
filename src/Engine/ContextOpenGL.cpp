#include "stdafx.h"
#if RENDER_OPENGL
#include "ContextOpenGL.h"
//=============================================================================
RenderContext::RenderContext()
{
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &pixel_buffer);

	int max_draw_buffers;
	glGetIntegerv(GL_MAX_DRAW_BUFFERS, &max_draw_buffers);

	for (int i = 0; i < max_draw_buffers; i++)
	{
		draw_buffers.push_back(GL_COLOR_ATTACHMENT0 + i);
	}

	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &max_vertex_attribs);
}
//=============================================================================
RenderContext::~RenderContext()
{
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &pixel_buffer);

	for (const auto& [state, objects_map] : sampler_states)
	{
		for (const auto& [type, object] : objects_map)
		{
			glDeleteSamplers(1, &object);
		}
	}
}
//=============================================================================
uint32_t RenderContext::GetBackbufferWidth()
{
	return !render_targets.empty() ? render_targets.at(0)->GetTexture()->GetWidth() : width;
}
//=============================================================================
uint32_t RenderContext::GetBackbufferHeight()
{
	return !render_targets.empty() ? render_targets.at(0)->GetTexture()->GetHeight() : height;
}
//=============================================================================
PixelFormat RenderContext::GetBackbufferFormat()
{
	return !render_targets.empty() ? render_targets.at(0)->GetTexture()->GetFormat() : PixelFormat::RGBA8UNorm;
}
//=============================================================================
#endif // RENDER_OPENGL