#pragma once

#if RENDER_D3D11

#include "RenderCoreD3D11.h"

class BufferD3D11
{
public:
	BufferD3D11(size_t size, D3D11_BIND_FLAG bind_flags);

	void Write(const void* memory, size_t size);

	const auto& GetD3D11Buffer() const { return mBuffer; }
	auto GetSize() const { return mSize; }

private:
	ComPtr<ID3D11Buffer> mBuffer;
	size_t mSize = 0;
};

class VertexBufferD3D11 : public BufferD3D11
{
public:
	VertexBufferD3D11(size_t size, size_t stride) : BufferD3D11(size, D3D11_BIND_VERTEX_BUFFER), mStride(stride) {}

	auto GetStride() const { return mStride; }
	void SetStride(size_t value) { mStride = value; }

private:
	size_t mStride = 0;
};

class IndexBufferD3D11 : public BufferD3D11
{
public:
	IndexBufferD3D11(size_t size, size_t stride) : BufferD3D11(size, D3D11_BIND_INDEX_BUFFER), mStride(stride) {}

	auto GetStride() const { return mStride; }
	void SetStride(size_t value) { mStride = value; }

private:
	size_t mStride = 0;
};

class UniformBufferD3D11 : public BufferD3D11
{
public:
	UniformBufferD3D11(size_t size) : BufferD3D11(size, D3D11_BIND_CONSTANT_BUFFER)
	{
		assert(size % 16 == 0);
	}
};

#endif // RENDER_D3D11