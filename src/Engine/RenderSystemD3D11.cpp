﻿#include "stdafx.h"
#if RENDER_D3D11
#include "RenderSystem.h"
#include "ContextD3D11.h"
#include "RenderTargetD3D11.h"
#include "BufferD3D11.h"
#include "ShaderD3D11.h"
//=============================================================================
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")
//=============================================================================
RenderContext gContext{};
//=============================================================================
bool RenderSystem::createAPI(const WindowPrivateData& data, const RenderSystemCreateInfo& createInfo)
{
	gContext.vsync = createInfo.vsync;

	ComPtr<IDXGIFactory6> dxgi_factory;
	CreateDXGIFactory1(IID_PPV_ARGS(dxgi_factory.GetAddressOf()));

	IDXGIAdapter1* adapter;
	auto gpu_preference = DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE;
	dxgi_factory->EnumAdapterByGpuPreference(0, gpu_preference, IID_PPV_ARGS(&adapter));

	DXGI_SWAP_CHAIN_DESC sd = {};
	sd.BufferCount = 2;
	sd.BufferDesc.Width = data.width;
	sd.BufferDesc.Height = data.height;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 5;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = data.hwnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

#if defined(_DEBUG)
	UINT flags = D3D11_CREATE_DEVICE_DEBUG;
#else
	UINT flags = 0;
#endif

	D3D11CreateDeviceAndSwapChain(adapter, D3D_DRIVER_TYPE_UNKNOWN, NULL, flags, NULL, 0,
		D3D11_SDK_VERSION, &sd, gContext.swapchain.GetAddressOf(), gContext.device.GetAddressOf(),
		NULL, gContext.context.GetAddressOf());

#if defined(_DEBUG)
	ComPtr<ID3D11InfoQueue> info_queue;
	gContext.device->QueryInterface(IID_PPV_ARGS(info_queue.GetAddressOf()));

	info_queue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
	info_queue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
	info_queue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_INFO, true);
	info_queue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_MESSAGE, true);
	info_queue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_WARNING, true);
#endif

	CreateMainRenderTarget(data.width, data.height);
	SetRenderTarget(nullptr, 0);

	return true;
}
//=============================================================================
void RenderSystem::destroyAPI()
{
	DestroyMainRenderTarget();
	// TODO: очистить gContext
}
//=============================================================================
void RenderSystem::resize(uint32_t width, uint32_t height)
{
	DestroyMainRenderTarget();
	gContext.swapchain->ResizeBuffers(0, (UINT)width, (UINT)height, DXGI_FORMAT_R8G8B8A8_UNORM, 0);
	CreateMainRenderTarget(width, height);
	SetRenderTarget(nullptr, 0);

	if (!gContext.viewport.has_value())
		gContext.viewport_dirty = true;
}
//=============================================================================
void RenderSystem::present()
{
	gContext.swapchain->Present(gContext.vsync ? 1 : 0, 0);
}
//=============================================================================
TextureHandle* RenderSystem::CreateTexture(uint32_t width, uint32_t height, PixelFormat format, uint32_t mip_count)
{
	auto texture = new TextureD3D11(width, height, format, mip_count);
	return (TextureHandle*)texture;
}
//=============================================================================
void RenderSystem::WriteTexturePixels(TextureHandle* handle, uint32_t width, uint32_t height, PixelFormat format, const void* memory, uint32_t mip_level, uint32_t offset_x, uint32_t offset_y)
{
	auto texture = (TextureD3D11*)handle;
	texture->Write(width, height, format, memory, mip_level, offset_x, offset_y);
}
//=============================================================================
void RenderSystem::GenerateMips(TextureHandle* handle)
{
	auto texture = (TextureD3D11*)handle;
	texture->GenerateMips();
}
//=============================================================================
void RenderSystem::DestroyTexture(TextureHandle* handle)
{
	auto texture = (TextureD3D11*)handle;
	delete texture;
}
//=============================================================================
RenderTargetHandle* RenderSystem::CreateRenderTarget(uint32_t width, uint32_t height, TextureHandle* texture_handle)
{
	auto texture = (TextureD3D11*)texture_handle;
	auto render_target = new RenderTargetD3D11(width, height, texture);
	return (RenderTargetHandle*)render_target;
}
//=============================================================================
void RenderSystem::DestroyRenderTarget(RenderTargetHandle* handle)
{
	auto render_target = (RenderTargetD3D11*)handle;
	delete render_target;
}
//=============================================================================
ShaderHandle* RenderSystem::CreateShader(const std::string& vertex_code, const std::string& fragment_code, const std::vector<std::string>& defines)
{
	auto shader = new ShaderD3D11(vertex_code, fragment_code, defines);
	return (ShaderHandle*)shader;
}
//=============================================================================
void RenderSystem::DestroyShader(ShaderHandle* handle)
{
	auto shader = (ShaderD3D11*)handle;
	delete shader;
}
//=============================================================================
VertexBufferHandle* RenderSystem::CreateVertexBuffer(size_t size, size_t stride)
{
	auto buffer = new VertexBufferD3D11(size, stride);
	return (VertexBufferHandle*)buffer;
}
//=============================================================================
void RenderSystem::DestroyVertexBuffer(VertexBufferHandle* handle)
{
	auto buffer = (VertexBufferD3D11*)handle;
	delete buffer;
}
//=============================================================================
void RenderSystem::WriteVertexBufferMemory(VertexBufferHandle* handle, const void* memory, size_t size, size_t stride)
{
	auto buffer = (VertexBufferD3D11*)handle;
	buffer->Write(memory, size);
	buffer->SetStride(stride);
}
//=============================================================================
IndexBufferHandle* RenderSystem::CreateIndexBuffer(size_t size, size_t stride)
{
	auto buffer = new IndexBufferD3D11(size, stride);
	return (IndexBufferHandle*)buffer;
}
//=============================================================================
void RenderSystem::DestroyIndexBuffer(IndexBufferHandle* handle)
{
	auto buffer = (IndexBufferD3D11*)handle;
	delete buffer;
}
//=============================================================================
void RenderSystem::WriteIndexBufferMemory(IndexBufferHandle* handle, const void* memory, size_t size, size_t stride)
{
	auto buffer = (IndexBufferD3D11*)handle;
	buffer->Write(memory, size);
	buffer->SetStride(stride);
}
//=============================================================================
UniformBufferHandle* RenderSystem::CreateUniformBuffer(size_t size)
{
	auto buffer = new UniformBufferD3D11(size);
	return (UniformBufferHandle*)buffer;
}
//=============================================================================
void RenderSystem::DestroyUniformBuffer(UniformBufferHandle* handle)
{
	auto buffer = (UniformBufferD3D11*)handle;
	delete buffer;
}
//=============================================================================
void RenderSystem::WriteUniformBufferMemory(UniformBufferHandle* handle, const void* memory, size_t size)
{
	auto buffer = (UniformBufferD3D11*)handle;
	buffer->Write(memory, size);
}
//=============================================================================
void RenderSystem::SetTopology(Topology topology)
{
	const static std::unordered_map<Topology, D3D11_PRIMITIVE_TOPOLOGY> TopologyMap = {
	{ Topology::PointList, D3D11_PRIMITIVE_TOPOLOGY_POINTLIST },
	{ Topology::LineList, D3D11_PRIMITIVE_TOPOLOGY_LINELIST },
	{ Topology::LineStrip, D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP },
	{ Topology::TriangleList, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST },
	{ Topology::TriangleStrip, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP }
	};

	gContext.context->IASetPrimitiveTopology(TopologyMap.at(topology));
}
//=============================================================================
void RenderSystem::SetViewport(std::optional<Viewport> viewport)
{
	gContext.viewport = viewport;
	gContext.viewport_dirty = true;
}
//=============================================================================
void RenderSystem::SetScissor(std::optional<Scissor> scissor)
{
	if (scissor.has_value())
	{
		auto value = scissor.value();

		D3D11_RECT rect;
		rect.left = static_cast<LONG>(value.position.x);
		rect.top = static_cast<LONG>(value.position.y);
		rect.right = static_cast<LONG>(value.position.x + value.size.x);
		rect.bottom = static_cast<LONG>(value.position.y + value.size.y);
		gContext.context->RSSetScissorRects(1, &rect);
	}

	gContext.rasterizer_state.scissor_enabled = scissor.has_value();
	gContext.rasterizer_state_dirty = true;
}
//=============================================================================
void RenderSystem::SetTexture(uint32_t binding, TextureHandle* handle)
{
	auto texture = (TextureD3D11*)handle;
	gContext.context->PSSetShaderResources((UINT)binding, 1, texture->GetD3D11ShaderResourceView().GetAddressOf());
	gContext.textures[binding] = texture;
}
//=============================================================================
void RenderSystem::SetTexture(uint32_t binding, const TextureHandle* handle)
{
	auto texture = (TextureD3D11*)handle;
	gContext.context->PSSetShaderResources((UINT)binding, 1, texture->GetD3D11ShaderResourceView().GetAddressOf());
	gContext.textures[binding] = texture;
}
//=============================================================================
void RenderSystem::SetRenderTarget(const RenderTarget** render_target, size_t count)
{
	if (count == 0)
	{
		gContext.context->OMSetRenderTargets(1, gContext.main_render_target->GetD3D11RenderTargetView().GetAddressOf(),
			gContext.main_render_target->GetD3D11DepthStencilView().Get());

		gContext.render_targets = { gContext.main_render_target };

		if (!gContext.viewport.has_value())
			gContext.viewport_dirty = true;

		return;
	}

	ComPtr<ID3D11ShaderResourceView> prev_shader_resource_view;
	gContext.context->PSGetShaderResources(0, 1, prev_shader_resource_view.GetAddressOf());

	std::vector<ID3D11RenderTargetView*> render_target_views;
	std::optional<ID3D11DepthStencilView*> depth_stencil_view;

	gContext.render_targets.clear();

	for (size_t i = 0; i < count; i++)
	{
		auto target = (RenderTargetD3D11*)(RenderTargetHandle*)*(RenderTarget*)render_target[i];

		if (prev_shader_resource_view.Get() == target->GetTexture()->GetD3D11ShaderResourceView().Get())
		{
			ID3D11ShaderResourceView* null[] = { NULL };
			gContext.context->PSSetShaderResources(0, 1, null); // remove old shader view
			// TODO: here we removing only binding 0, 
			// we should remove every binding with this texture
		}

		render_target_views.push_back(target->GetD3D11RenderTargetView().Get());

		if (!depth_stencil_view.has_value())
			depth_stencil_view = target->GetD3D11DepthStencilView().Get();

		gContext.render_targets.push_back(target);
	}

	gContext.context->OMSetRenderTargets((UINT)render_target_views.size(),
		render_target_views.data(), depth_stencil_view.value_or(nullptr));

	if (!gContext.viewport.has_value())
		gContext.viewport_dirty = true;
}
//=============================================================================
void RenderSystem::SetShader(ShaderHandle* handle)
{
	gContext.shader = (ShaderD3D11*)handle;
	gContext.shader_dirty = true;
	gContext.input_layouts_dirty = true;
}
//=============================================================================
void RenderSystem::SetShader(const ShaderHandle* handle)
{
	gContext.shader = (ShaderD3D11*)handle;
	gContext.shader_dirty = true;
	gContext.input_layouts_dirty = true;
}
//=============================================================================
void RenderSystem::SetInputLayout(const std::vector<InputLayout>& value)
{
	gContext.input_layouts = value;
	gContext.input_layouts_dirty = true;
}
//=============================================================================
void RenderSystem::SetVertexBuffer(const VertexBuffer** vertex_buffer, size_t count)
{
	std::vector<ID3D11Buffer*> buffers;
	std::vector<UINT> strides;
	std::vector<UINT> offsets;

	for (size_t i = 0; i < count; i++)
	{
		auto buffer = (VertexBufferD3D11*)(VertexBufferHandle*)*(VertexBuffer*)vertex_buffer[i];
		buffers.push_back(buffer->GetD3D11Buffer().Get());
		strides.push_back((UINT)buffer->GetStride());
		offsets.push_back(0);
	}

	gContext.context->IASetVertexBuffers(0, (UINT)buffers.size(), buffers.data(), strides.data(), offsets.data());
}
//=============================================================================
void RenderSystem::SetIndexBuffer(IndexBufferHandle* handle)
{
	auto buffer = (IndexBufferD3D11*)handle;
	auto stride = (UINT)buffer->GetStride();
	gContext.context->IASetIndexBuffer(buffer->GetD3D11Buffer().Get(), buffer->GetStride() == 2 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT, 0);
}
//=============================================================================
void RenderSystem::SetIndexBuffer(const IndexBufferHandle* handle)
{
	auto buffer = (IndexBufferD3D11*)handle;
	auto stride = (UINT)buffer->GetStride();
	gContext.context->IASetIndexBuffer(buffer->GetD3D11Buffer().Get(), buffer->GetStride() == 2 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT, 0);
}
//=============================================================================
void RenderSystem::SetUniformBuffer(uint32_t binding, UniformBufferHandle* handle)
{
	auto buffer = (UniformBufferD3D11*)handle;
	gContext.context->VSSetConstantBuffers(binding, 1, buffer->GetD3D11Buffer().GetAddressOf());
	gContext.context->PSSetConstantBuffers(binding, 1, buffer->GetD3D11Buffer().GetAddressOf());
}
//=============================================================================
void RenderSystem::SetBlendMode(const std::optional<BlendMode>& blend_mode)
{
	gContext.blend_mode = blend_mode;
	gContext.blend_mode_dirty = true;
}
//=============================================================================
void RenderSystem::SetDepthMode(const std::optional<DepthMode>& depth_mode)
{
	gContext.depth_stencil_state.depth_mode = depth_mode;
	gContext.depth_stencil_state_dirty = true;
}
//=============================================================================
void RenderSystem::SetStencilMode(const std::optional<StencilMode>& stencil_mode)
{
	gContext.depth_stencil_state.stencil_mode = stencil_mode;
	gContext.depth_stencil_state_dirty = true;
}
//=============================================================================
void RenderSystem::SetCullMode(CullMode cull_mode)
{
	gContext.rasterizer_state.cull_mode = cull_mode;
	gContext.rasterizer_state_dirty = true;
}
//=============================================================================
void RenderSystem::SetSampler(Sampler value)
{
	gContext.sampler_state.sampler = value;
	gContext.sampler_state_dirty = true;
}
//=============================================================================
void RenderSystem::SetTextureAddress(TextureAddress value)
{
	gContext.sampler_state.texture_address = value;
	gContext.sampler_state_dirty = true;
}
//=============================================================================
void RenderSystem::SetFrontFace(FrontFace value)
{
	gContext.rasterizer_state.front_face = value;
	gContext.rasterizer_state_dirty = true;
}
//=============================================================================
void RenderSystem::SetDepthBias(const std::optional<DepthBias> depth_bias)
{
	gContext.rasterizer_state.depth_bias = depth_bias;
	gContext.rasterizer_state_dirty = true;
}
//=============================================================================
void RenderSystem::Clear(const std::optional<glm::vec4>& color, const std::optional<float>& depth, const std::optional<uint8_t>& stencil)
{
	for (auto target : gContext.render_targets)
	{
		if (color.has_value())
		{
			gContext.context->ClearRenderTargetView(target->GetD3D11RenderTargetView().Get(), (float*)&color.value());
		}

		if (depth.has_value() || stencil.has_value())
		{
			UINT flags = 0;

			if (depth.has_value())
				flags |= D3D11_CLEAR_DEPTH;

			if (stencil.has_value())
				flags |= D3D11_CLEAR_STENCIL;

			gContext.context->ClearDepthStencilView(target->GetD3D11DepthStencilView().Get(), flags,
				depth.value_or(1.0f), stencil.value_or(0));
		}
	}
}
//=============================================================================
void RenderSystem::Draw(uint32_t vertex_count, uint32_t vertex_offset, uint32_t instance_count)
{
	EnsureGraphicsState(false);
	gContext.context->DrawInstanced((UINT)vertex_count, (UINT)instance_count, (UINT)vertex_offset, 0);
}
//=============================================================================
void RenderSystem::DrawIndexed(uint32_t index_count, uint32_t index_offset, uint32_t instance_count)
{
	EnsureGraphicsState(true);
	gContext.context->DrawIndexedInstanced((UINT)index_count, (UINT)instance_count, (UINT)index_offset, 0, 0);
}
//=============================================================================
void RenderSystem::ReadPixels(const glm::i32vec2& pos, const glm::i32vec2& size, TextureHandle* dst_texture_handle)
{
	auto dst_texture = (TextureD3D11*)dst_texture_handle;
	auto format = gContext.GetBackbufferFormat();

	assert(dst_texture->GetWidth() == size.x);
	assert(dst_texture->GetHeight() == size.y);
	assert(dst_texture->GetFormat() == format);

	if (size.x <= 0 || size.y <= 0)
		return;

	auto target = gContext.render_targets.at(0);

	ComPtr<ID3D11Resource> rtv_resource;
	target->GetD3D11RenderTargetView()->GetResource(rtv_resource.GetAddressOf());

	ComPtr<ID3D11Texture2D> rtv_texture;
	rtv_resource.As(&rtv_texture);

	D3D11_TEXTURE2D_DESC desc = { 0 };
	rtv_texture->GetDesc(&desc);
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

	D3D11_BOX box;
	box.left = src_x;
	box.right = src_x + src_w;
	box.top = src_y;
	box.bottom = src_y + src_h;
	box.front = 0;
	box.back = 1;

	if (pos.y < (int)back_h && pos.x < (int)back_w)
	{
		gContext.context->CopySubresourceRegion(dst_texture->GetD3D11Texture2D().Get(), 0, dst_x, dst_y, 0,
			rtv_resource.Get(), 0, &box);
	}
}
//=============================================================================
#endif // RENDER_D3D11