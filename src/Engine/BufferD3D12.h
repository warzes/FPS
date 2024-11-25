#pragma once

#if RENDER_D3D12

#include "RenderCoreD3D12.h"

class BufferD3D12
{
public:
	BufferD3D12(size_t size, D3D12_RESOURCE_STATES state);
	virtual ~BufferD3D12();

	void Write(const void* memory, size_t size);

	const auto& GetD3D12Buffer() const { return m_buffer; }
	auto GetSize() const { return m_size; }

private:
	ComPtr<ID3D12Resource> m_buffer;
	size_t m_size;
	D3D12_RESOURCE_STATES m_state;
};

class VertexBufferD3D12 final : public BufferD3D12
{
public:
	VertexBufferD3D12(size_t size, size_t stride) 
		: BufferD3D12(size, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER)
		, m_stride(stride)
	{
	}

	auto GetStride() const { return m_stride; }
	void SetStride(size_t value) { m_stride = value; }

private:
	size_t m_stride = 0;
};

class IndexBufferD3D12 : public BufferD3D12
{
public:
	IndexBufferD3D12(size_t size, size_t stride) 
		: BufferD3D12(size, D3D12_RESOURCE_STATE_INDEX_BUFFER)
		, m_stride(stride)
	{
	}

	auto GetStride() const { return m_stride; }
	void SetStride(size_t value) { m_stride = value; }

private:
	size_t m_stride = 0;
};

class UniformBufferD3D12 : public BufferD3D12
{
public:
	UniformBufferD3D12(size_t size) : BufferD3D12(DirectX::AlignUp((int)size, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT), D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER)
	{
	}
};

#endif // RENDER_D3D12