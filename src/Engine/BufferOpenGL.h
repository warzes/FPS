#pragma once

#if RENDER_OPENGL

#include "RenderCoreOpenGL.h"

class BufferGL
{
public:
	BufferGL(size_t size, GLenum type);
	~BufferGL();

	void Write(const void* memory, size_t size);

	auto GetGLBuffer() const { return m_buffer; }

private:
	GLuint m_buffer = 0;
	GLenum m_type = 0;
};

class VertexBufferGL : public BufferGL
{
public:
	VertexBufferGL(size_t size, size_t stride) : BufferGL(size, GL_ARRAY_BUFFER), m_stride(stride) {}

	auto GetStride() const { return m_stride; }
	void SetStride(size_t value) { m_stride = value; }

private:
	size_t m_stride = 0;
};

class IndexBufferGL : public BufferGL
{
public:
	IndexBufferGL(size_t size, size_t stride) : BufferGL(size, GL_ELEMENT_ARRAY_BUFFER), m_stride(stride) {}

	auto GetStride() const { return m_stride; }
	void SetStride(size_t value) { m_stride = value; }

private:
	size_t m_stride = 0;
};

class UniformBufferGL : public BufferGL
{
public:
	UniformBufferGL(size_t size) : BufferGL(size, GL_UNIFORM_BUFFER)
	{
		assert(size % 16 == 0);
	}
};


#endif // RENDER_OPENGL