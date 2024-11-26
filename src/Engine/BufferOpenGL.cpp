#include "stdafx.h"
#if RENDER_OPENGL
#include "BufferOpenGL.h"
//=============================================================================
BufferGL::BufferGL(size_t size, GLenum type) : m_type(type)
{
	glGenBuffers(1, &m_buffer);
	glBindBuffer(type, m_buffer);
	glBufferData(type, size, NULL, GL_DYNAMIC_DRAW);
}
//=============================================================================
BufferGL::~BufferGL()
{
	glDeleteBuffers(1, &m_buffer);
}
//=============================================================================
void BufferGL::Write(const void* memory, size_t size)
{
	glBindBuffer(m_type, m_buffer);
	glBufferData(m_type, size, memory, GL_DYNAMIC_DRAW);
}
//=============================================================================
#endif // RENDER_OPENGL