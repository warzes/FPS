#include "stdafx.h"
#if RENDER_D3D12
#include "ContextD3D12.h"
#include "RenderTargetD3D12.h"
#include "TextureD3D12.h"
#include "BufferD3D12.h"
#include "ShaderD3D12.h"
//=============================================================================
uint32_t RenderContext::GetBackbufferWidth()
{
	return !renderTargets.empty() ? renderTargets.at(0)->GetTexture()->GetWidth() : width;
}
//=============================================================================
uint32_t RenderContext::GetBackbufferHeight()
{
	return !renderTargets.empty() ? renderTargets.at(0)->GetTexture()->GetHeight() : height;
}
//=============================================================================
PixelFormat RenderContext::GetBackbufferFormat()
{
	return !renderTargets.empty() ? renderTargets.at(0)->GetTexture()->GetFormat() : PixelFormat::RGBA8UNorm;
}
//=============================================================================
void DestroyStaging(ComPtr<ID3D12DeviceChild> object)
{
	gContext.stagingObjects.push_back(object);
}
//=============================================================================
void ReleaseStaging()
{
	gContext.stagingObjects.clear();
}
//=============================================================================
void BeginCommandList(ID3D12CommandAllocator* cmd_alloc, ID3D12GraphicsCommandList* cmd_list)
{
	cmd_alloc->Reset();
	cmd_list->Reset(cmd_alloc, NULL);
}
//=============================================================================
void EndCommandList(ID3D12CommandQueue* cmd_queue, ID3D12GraphicsCommandList* cmd_list, bool wait_for)
{
	cmd_list->Close();
	cmd_queue->ExecuteCommandLists(1, CommandListCast(&cmd_list));
	if (wait_for)
	{
		ComPtr<ID3D12Fence> fence;
		DirectX::ThrowIfFailed(gContext.device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(fence.GetAddressOf())));

		DirectX::SetDebugObjectName(fence.Get(), L"ResourceUploadBatch");

		auto event = CreateEventEx(nullptr, nullptr, 0, EVENT_MODIFY_STATE | SYNCHRONIZE);

		DirectX::ThrowIfFailed(cmd_queue->Signal(fence.Get(), 1ULL));
		DirectX::ThrowIfFailed(fence->SetEventOnCompletion(1ULL, event));

		WaitForSingleObject(event, INFINITE);
	}
}
//=============================================================================
void OneTimeSubmit(std::function<void(ID3D12GraphicsCommandList*)> func)
{
	D3D12_COMMAND_QUEUE_DESC queue_desc = {};
	queue_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queue_desc.NodeMask = 1;

	ComPtr<ID3D12CommandQueue> cmd_queue;
	gContext.device->CreateCommandQueue(&queue_desc, IID_PPV_ARGS(cmd_queue.GetAddressOf()));

	ComPtr<ID3D12CommandAllocator> cmd_alloc;
	gContext.device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(cmd_alloc.GetAddressOf()));

	ComPtr<ID3D12GraphicsCommandList> cmd_list;
	gContext.device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, cmd_alloc.Get(), NULL, IID_PPV_ARGS(cmd_list.GetAddressOf()));

	cmd_list->Close();

	BeginCommandList(cmd_alloc.Get(), cmd_list.Get());
	func(cmd_list.Get());
	EndCommandList(cmd_queue.Get(), cmd_list.Get(), true);
}
//=============================================================================
ComPtr<ID3D12Resource> CreateBuffer(uint64_t size)
{
	ComPtr<ID3D12Resource> result;

	auto prop = CD3DX12_HEAP_PROPERTIES(D3D12_CPU_PAGE_PROPERTY_WRITE_COMBINE, D3D12_MEMORY_POOL_L0);
	auto desc = CD3DX12_RESOURCE_DESC::Buffer(size);

	gContext.device->CreateCommittedResource(&prop, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_COMMON, NULL, IID_PPV_ARGS(result.GetAddressOf()));

	return result;
}
//=============================================================================
void WaitForGpu()
{
	gContext.commandQueue->Signal(gContext.fence.Get(), gContext.fenceValue);
	gContext.fence->SetEventOnCompletion(gContext.fenceValue, gContext.fenceEvent);
	WaitForSingleObject(gContext.fenceEvent, INFINITE);
	gContext.fenceValue++;
	ReleaseStaging();
}
//=============================================================================
void CreateMainRenderTarget(uint32_t width, uint32_t height)
{
	D3D12_DESCRIPTOR_HEAP_DESC rtv_heap_desc = {};
	rtv_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtv_heap_desc.NumDescriptors = NUM_BACK_BUFFERS;
	rtv_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtv_heap_desc.NodeMask = 1;
	gContext.device->CreateDescriptorHeap(&rtv_heap_desc, IID_PPV_ARGS(gContext.frameRTVHeap.GetAddressOf()));

	auto rtv_increment_size = gContext.device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	auto rtv_heap_start = gContext.frameRTVHeap->GetCPUDescriptorHandleForHeapStart();

	for (UINT i = 0; i < NUM_BACK_BUFFERS; i++)
	{
		auto& frame = gContext.frames[i];

		ComPtr<ID3D12Resource> backbuffer;
		gContext.swapChain->GetBuffer(i, IID_PPV_ARGS(backbuffer.GetAddressOf()));

		frame.backBufferTexture = new TextureD3D12(width, height, PixelFormat::RGBA8UNorm, backbuffer);
		frame.rtvDescriptor = CD3DX12_CPU_DESCRIPTOR_HANDLE(rtv_heap_start, i, rtv_increment_size);
		frame.mainRenderTarget = new RenderTargetD3D12(width, height, frame.backBufferTexture, frame.rtvDescriptor);
	}

	gContext.width = width;
	gContext.height = height;

	gContext.frameIndex = gContext.swapChain->GetCurrentBackBufferIndex();
}
//=============================================================================
void DestroyMainRenderTarget()
{
	for (UINT i = 0; i < NUM_BACK_BUFFERS; i++)
	{
		delete gContext.frames[i].backBufferTexture;
		delete gContext.frames[i].mainRenderTarget;
	}
}
//=============================================================================
void EnsureViewport()
{
	if (!gContext.viewportDirty)
		return;

	gContext.viewportDirty = false;

	auto width = static_cast<float>(gContext.GetBackbufferWidth());
	auto height = static_cast<float>(gContext.GetBackbufferHeight());

	auto viewport = gContext.viewport.value_or(Viewport{ { 0.0f, 0.0f }, { width, height } });

	D3D12_VIEWPORT vp = {};
	vp.Width = viewport.size.x;
	vp.Height = viewport.size.y;
	vp.MinDepth = viewport.minDepth;
	vp.MaxDepth = viewport.maxDepth;
	vp.TopLeftX = viewport.position.x;
	vp.TopLeftY = viewport.position.y;
	gContext.commandList->RSSetViewports(1, &vp);
}
//=============================================================================
void EnsureScissor()
{
	if (!gContext.scissorDirty)
		return;

	gContext.scissorDirty = false;

	auto width = static_cast<float>(gContext.GetBackbufferWidth());
	auto height = static_cast<float>(gContext.GetBackbufferHeight());

	auto scissor = gContext.scissor.value_or(Scissor{ { 0.0f, 0.0f }, { width, height } });

	D3D12_RECT rect;
	rect.left = static_cast<LONG>(scissor.position.x);
	rect.top = static_cast<LONG>(scissor.position.y);
	rect.right = static_cast<LONG>(scissor.position.x + scissor.size.x);
	rect.bottom = static_cast<LONG>(scissor.position.y + scissor.size.y);
	gContext.commandList->RSSetScissorRects(1, &rect);
}
//=============================================================================
void EnsureTopology()
{
	if (!gContext.topologyDirty)
		return;

	gContext.topologyDirty = false;

	const static std::unordered_map<Topology, D3D_PRIMITIVE_TOPOLOGY> TopologyMap = {
		{ Topology::PointList, D3D_PRIMITIVE_TOPOLOGY_POINTLIST },
		{ Topology::LineList, D3D_PRIMITIVE_TOPOLOGY_LINELIST },
		{ Topology::LineStrip, D3D_PRIMITIVE_TOPOLOGY_LINESTRIP },
		{ Topology::TriangleList, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST },
		{ Topology::TriangleStrip, D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP }
	};

	auto topology = TopologyMap.at(gContext.topology);
	gContext.commandList->IASetPrimitiveTopology(topology);
}
//=============================================================================
void EnsureIndexBuffer()
{
	if (!gContext.indexBufferDirty)
		return;

	gContext.indexBufferDirty = false;

	D3D12_INDEX_BUFFER_VIEW buffer_view = {};
	buffer_view.BufferLocation = gContext.indexBuffer->GetD3D12Buffer()->GetGPUVirtualAddress();
	buffer_view.SizeInBytes = (UINT)gContext.indexBuffer->GetSize();
	buffer_view.Format = gContext.indexBuffer->GetStride() == 2 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;

	gContext.commandList->IASetIndexBuffer(&buffer_view);
}
//=============================================================================
void EnsureVertexBuffer()
{
	if (!gContext.vertexBuffersDirty)
		return;

	gContext.vertexBuffersDirty = false;

	std::vector<D3D12_VERTEX_BUFFER_VIEW> buffer_views;

	for (auto vertex_buffer : gContext.vertexBuffers)
	{
		auto buffer_view = D3D12_VERTEX_BUFFER_VIEW{
			.BufferLocation = vertex_buffer->GetD3D12Buffer()->GetGPUVirtualAddress(),
			.SizeInBytes = (UINT)vertex_buffer->GetSize(),
			.StrideInBytes = (UINT)vertex_buffer->GetStride()
		};

		buffer_views.push_back(buffer_view);
	}

	gContext.commandList->IASetVertexBuffers(0, (UINT)buffer_views.size(), buffer_views.data());
}
//=============================================================================
void EnsureGraphicsState(bool draw_indexed)
{
	auto shader = gContext.pipelineState.shader;
	assert(shader);

	if (!gContext.pipelineStates.contains(gContext.pipelineState))
	{
		auto pipeline_state = CreateGraphicsPipelineState(gContext.pipelineState);
		gContext.pipelineStates[gContext.pipelineState] = pipeline_state;
	}

	auto targets = gContext.renderTargets;
	std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> rtv_descriptors;

	if (targets.empty())
	{
		targets = { gContext.GetCurrentFrame().mainRenderTarget };
		rtv_descriptors = { gContext.GetCurrentFrame().rtvDescriptor };
	}
	else
	{
		for (auto target : targets)
		{
			rtv_descriptors.push_back(target->GetRtvHeap()->GetCPUDescriptorHandleForHeapStart());
		}
	}

	for (auto target : targets)
	{
		target->GetTexture()->EnsureState(gContext.commandList.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET);
	}

	auto dsv_descriptor = targets.at(0)->GetDsvHeap()->GetCPUDescriptorHandleForHeapStart();

	gContext.commandList->OMSetRenderTargets((UINT)rtv_descriptors.size(), rtv_descriptors.data(), FALSE, &dsv_descriptor);

	auto pipeline_state = gContext.pipelineStates.at(gContext.pipelineState).Get();

	gContext.commandList->SetPipelineState(pipeline_state);
	gContext.commandList->SetGraphicsRootSignature(shader->GetRootSignature().Get());
	gContext.commandList->SetDescriptorHeaps(1, gContext.descriptorHeap.GetAddressOf());

	const auto& required_typed_descriptor_bindings = shader->GetRequiredTypedDescriptorBindings();
	const auto& binding_to_root_index_map = shader->GetBindingToRootIndexMap();

	for (const auto& [type, required_descriptor_bindings] : required_typed_descriptor_bindings)
	{
		for (const auto& [binding, descriptor] : required_descriptor_bindings)
		{
			auto root_index = binding_to_root_index_map.at(binding);
			if (type == ShaderReflection::DescriptorType::CombinedImageSampler)
			{
				const auto& texture = gContext.textures.at(binding);
				texture->EnsureState(gContext.commandList.Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
				gContext.commandList->SetGraphicsRootDescriptorTable(root_index, texture->GetGpuDescriptorHandle());
			}
			else if (type == ShaderReflection::DescriptorType::UniformBuffer)
			{
				auto uniform_buffer = gContext.uniformBuffers.at(binding);
				gContext.commandList->SetGraphicsRootConstantBufferView(root_index, uniform_buffer->GetD3D12Buffer()->GetGPUVirtualAddress());
			}
			else
			{
				assert(false);
			}
		}
	}

	EnsureViewport();
	EnsureScissor();
	EnsureTopology();

	if (draw_indexed)
		EnsureIndexBuffer();

	EnsureVertexBuffer();
}
//=============================================================================
void Begin()
{
	BeginCommandList(gContext.commandAllocator.Get(), gContext.commandList.Get());

	gContext.topologyDirty = true;
	gContext.viewportDirty = true;
	gContext.scissorDirty = true;
	gContext.indexBufferDirty = true;
	gContext.vertexBuffersDirty = true;
}
//=============================================================================
void End()
{
	gContext.GetCurrentFrame().backBufferTexture->EnsureState(gContext.commandList.Get(), D3D12_RESOURCE_STATE_PRESENT);
	EndCommandList(gContext.commandQueue.Get(), gContext.commandList.Get(), false);
}
//=============================================================================
#endif // RENDER_D3D12