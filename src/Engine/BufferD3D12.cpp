#include "stdafx.h"
#if RENDER_D3D12
#include "BufferD3D12.h"
#include "ContextD3D12.h"
//=============================================================================
BufferD3D12::BufferD3D12(size_t size, D3D12_RESOURCE_STATES state) 
	: m_size(size)
	, m_state(state)
{
	m_buffer = CreateBuffer(size);

	if (state != D3D12_RESOURCE_STATE_COMMON)
	{
		OneTimeSubmit([&](ID3D12GraphicsCommandList* cmdlist) {
			DirectX::TransitionResource(cmdlist, m_buffer.Get(), D3D12_RESOURCE_STATE_COMMON, state);
			});
	}
}
//=============================================================================
BufferD3D12::~BufferD3D12()
{
	DestroyStaging(m_buffer);
}
//=============================================================================
void BufferD3D12::Write(const void* memory, size_t size)
{
	auto staging_buffer = CreateBuffer(size);

	void* cpu_memory = nullptr;
	staging_buffer->Map(0, NULL, &cpu_memory);
	memcpy(cpu_memory, memory, size);
	staging_buffer->Unmap(0, NULL);

	auto barrier = DirectX::ScopedBarrier(gContext.commandList.Get(), {
		CD3DX12_RESOURCE_BARRIER::Transition(m_buffer.Get(), m_state, D3D12_RESOURCE_STATE_COPY_DEST)
		});
	gContext.commandList->CopyBufferRegion(m_buffer.Get(), 0, staging_buffer.Get(), 0, (UINT64)size);
	DestroyStaging(staging_buffer);
}
//=============================================================================
#endif // RENDER_D3D12