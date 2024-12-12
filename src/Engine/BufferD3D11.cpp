#include "stdafx.h"
#if RENDER_D3D11
#include "BufferD3D11.h"
#include "ContextD3D11.h"
#include "Log.h"
//=============================================================================
BufferD3D11::BufferD3D11(size_t size, D3D11_BIND_FLAG bindFlags) : m_size(size)
{
	auto desc = CD3D11_BUFFER_DESC((UINT)size, bindFlags, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);
	HRESULT hr = gContext.device->CreateBuffer(&desc, nullptr, m_buffer.GetAddressOf());
	if (FAILED(hr))
	{
		Fatal("CreateBuffer() failed: " + DXErrorToStr(hr));
		return;
	}
}
//=============================================================================
void BufferD3D11::Write(const void* memory, size_t size)
{
	assert(size <= m_size);
	D3D11_MAPPED_SUBRESOURCE resource;
	HRESULT hr = gContext.context->Map(m_buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
	if (FAILED(hr))
	{
		Fatal("Map() failed: " + DXErrorToStr(hr));
		return;
	}
	memcpy(resource.pData, memory, size);
	gContext.context->Unmap(m_buffer.Get(), 0);
}
//=============================================================================
#endif // RENDER_D3D11