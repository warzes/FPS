#pragma once

#if RENDER_D3D11

#include "RHICoreD3D11.h"

class BufferD3D11
{
public:
	BufferD3D11() = delete;
	BufferD3D11(size_t size, D3D11_BIND_FLAG bindFlags);

	void Write(const void* memory, size_t size);

	const auto& GetD3D11Buffer() const { return m_buffer; }
	auto        GetSize() const { return m_size; }

private:
	ComPtr<ID3D11Buffer> m_buffer;
	size_t m_size = 0;
};

class VertexBufferD3D11 final : public BufferD3D11
{
public:
	VertexBufferD3D11() = delete;
	VertexBufferD3D11(size_t size, size_t stride) : BufferD3D11(size, D3D11_BIND_VERTEX_BUFFER), m_stride(stride) {}

	auto GetStride() const { return m_stride; }
	void SetStride(size_t value) { m_stride = value; }

private:
	size_t m_stride = 0;
};

class IndexBufferD3D11 final : public BufferD3D11
{
public:
	IndexBufferD3D11() = delete;
	IndexBufferD3D11(size_t size, size_t stride) : BufferD3D11(size, D3D11_BIND_INDEX_BUFFER), m_stride(stride) {}

	auto GetStride() const { return m_stride; }
	void SetStride(size_t value) { m_stride = value; }

private:
	size_t m_stride = 0;
};

class UniformBufferD3D11 final : public BufferD3D11
{
public:
	UniformBufferD3D11(size_t size) : BufferD3D11(size, D3D11_BIND_CONSTANT_BUFFER)
	{
		assert(size % 16 == 0);
	}
};

#endif // RENDER_D3D11