#include "stdafx.h"
#if RENDER_D3D11
#include "BufferD3D11.h"
#include "ContextD3D11.h"
//=============================================================================
BufferD3D11::BufferD3D11(size_t size, D3D11_BIND_FLAG bind_flags) : mSize(size)
{
	auto desc = CD3D11_BUFFER_DESC((UINT)size, bind_flags, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);
	gContext.device->CreateBuffer(&desc, NULL, mBuffer.GetAddressOf());
}
//=============================================================================
void BufferD3D11::Write(const void* memory, size_t size)
{
	assert(size <= mSize);
	D3D11_MAPPED_SUBRESOURCE resource;
	gContext.context->Map(mBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
	memcpy(resource.pData, memory, size);
	gContext.context->Unmap(mBuffer.Get(), 0);
}
//=============================================================================
#endif // RENDER_D3D11