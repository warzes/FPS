#include "stdafx.h"
#if RENDER_D3D12
#include "RHIBackend.h"
#include "ContextD3D12.h"
#include "ShaderD3D12.h"
#include "TextureD3D12.h"
#include "RenderTargetD3D12.h"
#include "BufferD3D12.h"
#include "Log.h"
#include "RHIResources.h"
//=============================================================================
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")
//=============================================================================
RenderContext gContext{};
//=============================================================================
bool RHIBackend::CreateAPI(const WindowData& data, const RenderSystemCreateInfo& createInfo)
{
#if RHI_VALIDATION_ENABLED
	ComPtr<ID3D12Debug6> debug6;
	D3D12GetDebugInterface(IID_PPV_ARGS(debug6.GetAddressOf()));
	if (debug6)
	{
		debug6->EnableDebugLayer();
		debug6->SetEnableAutoName(true);
		debug6->SetEnableGPUBasedValidation(true);
		debug6->SetEnableSynchronizedCommandQueueValidation(true);
		debug6->SetForceLegacyBarrierValidation(true);
	}
	else // TODO: пока так, на windows 10 у меня только ID3D12Debug3
	{
		ComPtr<ID3D12Debug3> debug3;
		D3D12GetDebugInterface(IID_PPV_ARGS(debug3.GetAddressOf()));
		debug3->EnableDebugLayer();
		debug3->SetEnableGPUBasedValidation(true);
		debug3->SetEnableSynchronizedCommandQueueValidation(true);
	}
#endif

	ComPtr<IDXGIFactory6> dxgi_factory;
	CreateDXGIFactory1(IID_PPV_ARGS(dxgi_factory.GetAddressOf()));

	IDXGIAdapter1* adapter;
	auto gpu_preference = DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE;
	dxgi_factory->EnumAdapterByGpuPreference(0, gpu_preference, IID_PPV_ARGS(&adapter));

	D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(gContext.device.GetAddressOf()));

#if RHI_VALIDATION_ENABLED
	ComPtr<ID3D12InfoQueue> info_queue;
	gContext.device.As(&info_queue);
	info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
	info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
	info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);

	std::vector<D3D12_MESSAGE_ID> filtered_messages = {
		D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,
		D3D12_MESSAGE_ID_CLEARDEPTHSTENCILVIEW_MISMATCHINGCLEARVALUE,
		D3D12_MESSAGE_ID_DRAW_EMPTY_SCISSOR_RECTANGLE
	};

	D3D12_INFO_QUEUE_FILTER filter = {};
	filter.DenyList.NumIDs = (UINT)filtered_messages.size();
	filter.DenyList.pIDList = filtered_messages.data();
	info_queue->AddStorageFilterEntries(&filter);
#endif

	D3D12_COMMAND_QUEUE_DESC queue_desc = {};
	queue_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queue_desc.NodeMask = 1;
	gContext.device->CreateCommandQueue(&queue_desc, IID_PPV_ARGS(gContext.commandQueue.GetAddressOf()));

	gContext.device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(gContext.commandAllocator.GetAddressOf()));

	gContext.device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, gContext.commandAllocator.Get(), NULL, IID_PPV_ARGS(gContext.commandList.GetAddressOf()));

	gContext.commandList->Close();

	D3D12_DESCRIPTOR_HEAP_DESC heap_desc = {};
	heap_desc.NumDescriptors = 1000; // TODO: make more dynamic
	heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	gContext.device->CreateDescriptorHeap(&heap_desc, IID_PPV_ARGS(gContext.descriptorHeap.GetAddressOf()));

	gContext.descriptorHandleIncrementSize = gContext.device->GetDescriptorHandleIncrementSize(heap_desc.Type);

	gContext.descriptorHeapCPUHandle = gContext.descriptorHeap->GetCPUDescriptorHandleForHeapStart();
	gContext.descriptorHeapGPUHandle = gContext.descriptorHeap->GetGPUDescriptorHandleForHeapStart();

	DXGI_SWAP_CHAIN_DESC1 swapchain_desc = {};
	swapchain_desc.BufferCount = RHI_BACKBUFFER_COUNT;
	swapchain_desc.Width = data.width;
	swapchain_desc.Height = data.height;
	swapchain_desc.Format = MainRenderTargetColorAttachmentFormat;
	swapchain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapchain_desc.SampleDesc.Count = 1;
	swapchain_desc.SampleDesc.Quality = 0;
	swapchain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapchain_desc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	swapchain_desc.Scaling = DXGI_SCALING_NONE;
	swapchain_desc.Stereo = FALSE;

	DXGI_SWAP_CHAIN_FULLSCREEN_DESC fs_swapchain_desc = { 0 };
	fs_swapchain_desc.Windowed = TRUE;

	ComPtr<IDXGISwapChain1> swapChain;
	dxgi_factory->CreateSwapChainForHwnd(gContext.commandQueue.Get(), data.hwnd, &swapchain_desc, &fs_swapchain_desc, NULL, swapChain.GetAddressOf());

	swapChain.As(&gContext.swapChain);

	gContext.device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(gContext.fence.GetAddressOf()));
	gContext.fenceValue = 1;
	gContext.fenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	assert(gContext.fenceEvent != NULL);

	gContext.pipelineState.colorAttachmentFormats = { MainRenderTargetColorAttachmentFormat };
	gContext.pipelineState.depthStencilFormat = MainRenderTargetDepthStencilAttachmentFormat;

	CreateMainRenderTarget(data.width, data.height);
	RenderBegin();

	return true;
}
//=============================================================================
void RHIBackend::DestroyAPI()
{
	RenderEnd();
	DestroyMainRenderTarget();
	WaitForGpu();
	// TODO: очистить gContext
}
//=============================================================================
void RHIBackend::ResizeFrameBuffer(uint32_t width, uint32_t height)
{
	RenderEnd();
	DestroyMainRenderTarget();
	WaitForGpu();
	gContext.swapChain->ResizeBuffers(RHI_BACKBUFFER_COUNT, width, height, MainRenderTargetColorAttachmentFormat, 0);
	CreateMainRenderTarget(width, height);
	RenderBegin();

	if (!gContext.viewport.has_value())
		gContext.viewportDirty = true;

	if (!gContext.scissor.has_value())
		gContext.scissorDirty = true;
}
//=============================================================================
void RHIBackend::Present()
{
	RenderEnd();
	bool vsync = false;
	gContext.swapChain->Present(vsync ? 1 : 0, 0);
	gContext.frameIndex = gContext.swapChain->GetCurrentBackBufferIndex();
	WaitForGpu();
	RenderBegin();
}
//=============================================================================
void RHIBackend::Clear(const std::optional<glm::vec4>& color, const std::optional<float>& depth, const std::optional<uint8_t>& stencil)
{
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

	if (color.has_value())
	{
		for (auto rtv_descriptor : rtv_descriptors)
		{
			gContext.commandList->ClearRenderTargetView(rtv_descriptor, (float*)&color.value(), 0, NULL);
		}
	}

	if (depth.has_value() || stencil.has_value())
	{
		D3D12_CLEAR_FLAGS flags = {};

		if (depth.has_value())
			flags |= D3D12_CLEAR_FLAG_DEPTH;

		if (stencil.has_value())
			flags |= D3D12_CLEAR_FLAG_STENCIL;

		gContext.commandList->ClearDepthStencilView(dsv_descriptor, flags, depth.value_or(1.0f), stencil.value_or(0), 0, NULL);
	}
}
//=============================================================================
void RHIBackend::Draw(uint32_t vertexCount, uint32_t vertexOffset, uint32_t instanceCount)
{
	EnsureGraphicsState(false);
	gContext.commandList->DrawInstanced((UINT)vertexCount, (UINT)instanceCount, (UINT)vertexOffset, 0);
}
//=============================================================================
void RHIBackend::DrawIndexed(uint32_t indexCount, uint32_t indexOffset, uint32_t instanceCount)
{
	EnsureGraphicsState(true);
	gContext.commandList->DrawIndexedInstanced((UINT)indexCount, (UINT)instanceCount, (UINT)indexOffset, 0, 0);
}
//=============================================================================
void RHIBackend::ReadPixels(const glm::i32vec2& pos, const glm::i32vec2& size, TextureHandle* dstTextureHandle)
{
	auto dst_texture = (TextureD3D12*)dstTextureHandle;
	auto format = gContext.GetBackBufferFormat();

	assert(dst_texture->GetWidth() == size.x);
	assert(dst_texture->GetHeight() == size.y);
	assert(dst_texture->GetFormat() == format);

	if (size.x <= 0 || size.y <= 0)
		return;

	auto src_texture = !gContext.renderTargets.empty() ?
		gContext.renderTargets.at(0)->GetTexture() :
		gContext.GetCurrentFrame().mainRenderTarget->GetTexture();

	auto desc = src_texture->GetD3D12Texture()->GetDesc();

	auto back_w = desc.Width;
	auto back_h = desc.Height;

	auto src_x = (UINT)pos.x;
	auto src_y = (UINT)pos.y;
	auto src_w = (UINT)size.x;
	auto src_h = (UINT)size.y;

	UINT dst_x = 0;
	UINT dst_y = 0;

	if (pos.x < 0)
	{
		src_x = 0;
		if (-pos.x > size.x)
			src_w = 0;
		else
			src_w += pos.x;

		dst_x = -pos.x;
	}

	if (pos.y < 0)
	{
		src_y = 0;
		if (-pos.y > size.y)
			src_h = 0;
		else
			src_h += pos.y;

		dst_y = -pos.y;
	}

	if (pos.y >= (int)back_h || pos.x >= (int)back_w)
		return;

	src_w = glm::min(src_w, (UINT)back_w);
	src_h = glm::min(src_h, (UINT)back_h);

	D3D12_BOX box;
	box.left = src_x;
	box.right = src_x + src_w;
	box.top = src_y;
	box.bottom = src_y + src_h;
	box.front = 0;
	box.back = 1;

	src_texture->EnsureState(gContext.commandList.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE);
	dst_texture->EnsureState(gContext.commandList.Get(), D3D12_RESOURCE_STATE_COPY_DEST);

	auto src_location = CD3DX12_TEXTURE_COPY_LOCATION(src_texture->GetD3D12Texture().Get(), 0);
	auto dst_location = CD3DX12_TEXTURE_COPY_LOCATION(dst_texture->GetD3D12Texture().Get(), 0);

	gContext.commandList->CopyTextureRegion(&dst_location, dst_x, dst_y, 0, &src_location, &box);
}
//=============================================================================
ShaderHandle* RHIBackend::CreateShader(const std::string& vertexCode, const std::string& fragmentCode, const std::vector<std::string>& defines)
{
	auto shader = new ShaderD3D12(vertexCode, fragmentCode, defines);
	return (ShaderHandle*)shader;
}
//=============================================================================
void RHIBackend::DestroyShader(ShaderHandle* handle)
{
	auto shader = (ShaderD3D12*)handle;
	delete shader;
}
//=============================================================================
TextureHandle* RHIBackend::CreateTexture(uint32_t width, uint32_t height, PixelFormat format, uint32_t mip_count)
{
	auto texture = new TextureD3D12(width, height, format, mip_count);
	return (TextureHandle*)texture;
}
//=============================================================================
void RHIBackend::WriteTexturePixels(TextureHandle* handle, uint32_t width, uint32_t height, PixelFormat format, const void* memory, uint32_t mip_level, uint32_t offset_x, uint32_t offset_y)
{
	auto texture = (TextureD3D12*)handle;
	texture->Write(width, height, format, memory, mip_level, offset_x, offset_y);
}
//=============================================================================
void RHIBackend::GenerateMips(TextureHandle* handle)
{
	auto texture = (TextureD3D12*)handle;
	texture->GenerateMips();
}
//=============================================================================
void RHIBackend::DestroyTexture(TextureHandle* handle)
{
	auto texture = (TextureD3D12*)handle;
	delete texture;
}
//=============================================================================
RenderTargetHandle* RHIBackend::CreateRenderTarget(uint32_t width, uint32_t height, TextureHandle* texture_handle)
{
	auto texture = (TextureD3D12*)texture_handle;
	auto render_target = new RenderTargetD3D12(width, height, texture);
	return (RenderTargetHandle*)render_target;
}
//=============================================================================
void RHIBackend::DestroyRenderTarget(RenderTargetHandle* handle)
{
	auto render_target = (RenderTargetD3D12*)handle;
	delete render_target;
}
//=============================================================================
VertexBufferHandle* RHIBackend::CreateVertexBuffer(size_t size, size_t stride)
{
	auto buffer = new VertexBufferD3D12(size, stride);
	return (VertexBufferHandle*)buffer;
}
//=============================================================================
void RHIBackend::DestroyVertexBuffer(VertexBufferHandle* handle)
{
	auto buffer = (VertexBufferD3D12*)handle;
	delete buffer;
}
//=============================================================================
void RHIBackend::WriteVertexBufferMemory(VertexBufferHandle* handle, const void* memory, size_t size, size_t stride)
{
	auto buffer = (VertexBufferD3D12*)handle;
	buffer->Write(memory, size);
	buffer->SetStride(stride);

	for (auto vertex_buffer : gContext.vertexBuffers)
	{
		if (vertex_buffer != buffer)
			continue;

		gContext.vertexBuffersDirty = true;
		break;
	}
}
//=============================================================================
IndexBufferHandle* RHIBackend::CreateIndexBuffer(size_t size, size_t stride)
{
	auto buffer = new IndexBufferD3D12(size, stride);
	return (IndexBufferHandle*)buffer;
}
//=============================================================================
void RHIBackend::DestroyIndexBuffer(IndexBufferHandle* handle)
{
	auto buffer = (IndexBufferD3D12*)handle;
	delete buffer;
}
//=============================================================================
void RHIBackend::WriteIndexBufferMemory(IndexBufferHandle* handle, const void* memory, size_t size, size_t stride)
{
	auto buffer = (IndexBufferD3D12*)handle;
	buffer->Write(memory, size);
	buffer->SetStride(stride);

	if (gContext.indexBuffer == buffer)
		gContext.indexBufferDirty = true;
}
//=============================================================================
UniformBufferHandle* RHIBackend::CreateUniformBuffer(size_t size)
{
	auto buffer = new UniformBufferD3D12(size);
	return (UniformBufferHandle*)buffer;
}
//=============================================================================
void RHIBackend::DestroyUniformBuffer(UniformBufferHandle* handle)
{
	auto buffer = (UniformBufferD3D12*)handle;
	delete buffer;
}
//=============================================================================
void RHIBackend::WriteUniformBufferMemory(UniformBufferHandle* handle, const void* memory, size_t size)
{
	auto buffer = (UniformBufferD3D12*)handle;
	buffer->Write(memory, size);
}
//=============================================================================
void RHIBackend::SetTopology(Topology topology)
{
	gContext.topology = topology;
	gContext.topologyDirty = true;
	gContext.pipelineState.topologyKind = GetTopologyKind(topology);
}
//=============================================================================
void RHIBackend::SetViewport(std::optional<Viewport> viewport)
{
	gContext.viewport = viewport;
	gContext.viewportDirty = true;
}
//=============================================================================
void RHIBackend::SetScissor(std::optional<Scissor> scissor)
{
	gContext.scissor = scissor;
	gContext.scissorDirty = true;
}
//=============================================================================
void RHIBackend::SetBlendMode(const std::optional<BlendMode>& blend_mode)
{
	gContext.pipelineState.blendMode = blend_mode;
}
//=============================================================================
void RHIBackend::SetDepthMode(const std::optional<DepthMode>& depth_mode)
{
	gContext.pipelineState.depthMode = depth_mode;
}
//=============================================================================
void RHIBackend::SetStencilMode(const std::optional<StencilMode>& stencil_mode)
{

}
//=============================================================================
void RHIBackend::SetCullMode(CullingMode cull_mode)
{
	gContext.pipelineState.rasterizerState.cullMode = cull_mode;
}
//=============================================================================
void RHIBackend::SetRasterizerState(const RasterizerState& state)
{
}
//=============================================================================
void RHIBackend::SetSamplerState(const SamplerState& state)
{

}
//=============================================================================
void RHIBackend::SetSamplerFilter(Filter value)
{

}
//=============================================================================
void RHIBackend::SetTextureAddress(TextureAddress value)
{

}
//=============================================================================
void RHIBackend::SetFrontFace(FrontFace value)
{

}
//=============================================================================
void RHIBackend::SetDepthBias(const std::optional<DepthBias> depth_bias)
{

}
//=============================================================================
void RHIBackend::SetShader(ShaderHandle* handle)
{
	gContext.pipelineState.shader = (ShaderD3D12*)handle;
}
//=============================================================================
void RHIBackend::SetInputLayout(const std::vector<InputLayout>& value)
{
	gContext.pipelineState.inputLayouts = value;
}
//=============================================================================
void RHIBackend::SetTexture(uint32_t binding, TextureHandle* handle)
{
	gContext.textures[binding] = (TextureD3D12*)handle;
}
//=============================================================================
void RHIBackend::SetRenderTarget(const RenderTarget** render_target, size_t count)
{
	std::vector<RenderTargetD3D12*> render_targets;
	std::vector<DXGI_FORMAT> color_attachment_formats;
	std::optional<DXGI_FORMAT> depth_stencil_format;

	if (count == 0)
	{
		color_attachment_formats = { MainRenderTargetColorAttachmentFormat };
		depth_stencil_format = MainRenderTargetDepthStencilAttachmentFormat;
	}
	else
	{
		for (size_t i = 0; i < count; i++)
		{
			auto target = (RenderTargetD3D12*)(RenderTargetHandle*)*(RenderTarget*)render_target[i];

			render_targets.push_back(target);
			color_attachment_formats.push_back(PixelFormatMap.at(target->GetTexture()->GetFormat()));

			if (!depth_stencil_format.has_value())
				depth_stencil_format = target->GetDepthStencilFormat();
		}
	}

	gContext.pipelineState.colorAttachmentFormats = color_attachment_formats;
	gContext.pipelineState.depthStencilFormat = depth_stencil_format;
	gContext.renderTargets = render_targets;

	if (!gContext.viewport.has_value())
		gContext.viewportDirty = true;

	if (!gContext.scissor.has_value())
		gContext.scissorDirty = true;
}
//=============================================================================
void RHIBackend::SetVertexBuffer(const VertexBuffer** vertex_buffer, size_t count)
{
	gContext.vertexBuffers.clear();
	for (size_t i = 0; i < count; i++)
	{
		auto buffer = (VertexBufferD3D12*)(VertexBufferHandle*)*(VertexBuffer*)vertex_buffer[i];
		gContext.vertexBuffers.push_back(buffer);
	}
	gContext.vertexBuffersDirty = true;
}
//=============================================================================
void RHIBackend::SetIndexBuffer(IndexBufferHandle* handle)
{
	gContext.indexBuffer = (IndexBufferD3D12*)handle;
	gContext.indexBufferDirty = true;
}
//=============================================================================
void RHIBackend::SetUniformBuffer(uint32_t binding, UniformBufferHandle* handle)
{
	gContext.uniformBuffers[binding] = (UniformBufferD3D12*)handle;
}
//=============================================================================
#endif // RENDER_D3D12