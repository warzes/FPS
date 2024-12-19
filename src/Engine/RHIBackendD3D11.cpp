#include "stdafx.h"
#if RENDER_D3D11
#include "RHIBackend.h"
#include "RHICoreD3D11.h"
#include "ContextD3D11.h"
#include "ShaderD3D11.h"
#include "TextureD3D11.h"
#include "RenderTargetD3D11.h"
#include "BufferD3D11.h"
#include "Log.h"
#include "RHIResources.h"
//=============================================================================
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")
//=============================================================================
RenderContext gContext{};
//=============================================================================
bool RHIBackend::CreateAPI(const WindowData& data, const RenderSystemCreateInfo& createInfo)
{
	gContext.vsync = createInfo.vsync;

	HRESULT hr = E_FAIL;

	ComPtr<IDXGIFactory7> dxgiFactory;
	hr = CreateDXGIFactory2(0, IID_PPV_ARGS(dxgiFactory.GetAddressOf()));
	if (FAILED(hr))
	{
		Fatal("CreateDXGIFactory2() failed: " + DXErrorToStr(hr));
		return false;
	}

	hr = dxgiFactory->EnumAdapterByGpuPreference(0, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(gContext.adapter.GetAddressOf()));
	if (FAILED(hr))
	{
		Fatal("EnumAdapterByGpuPreference() failed: " + DXErrorToStr(hr));
		return false;
	}

#if RHI_VALIDATION_ENABLED
	const UINT creationFlags = D3D11_CREATE_DEVICE_DEBUG;
#else
	const UINT creationFlags = 0;
#endif

	const D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_1;

	ComPtr<ID3D11Device> device = nullptr;
	ComPtr<ID3D11DeviceContext> context = nullptr;
	hr = D3D11CreateDevice(gContext.adapter.Get(), D3D_DRIVER_TYPE_UNKNOWN, nullptr, creationFlags, &featureLevel, 1, D3D11_SDK_VERSION, device.ReleaseAndGetAddressOf(), nullptr, context.ReleaseAndGetAddressOf());
	if (FAILED(hr))
	{
		Fatal("D3D11CreateDevice() failed: " + DXErrorToStr(hr));
		return false;
	}
	hr = device.As(&gContext.device);
	if (FAILED(hr))
	{
		Fatal("ID3D11Device as ID3D11Device5 failed: " + DXErrorToStr(hr));
		return false;
	}
	hr = context.As(&gContext.context);
	if (FAILED(hr))
	{
		Fatal("ID3D11DeviceContext as ID3D11DeviceContext4 failed: " + DXErrorToStr(hr));
		return false;
	}
	hr = context.As(&gContext.annotation);
	if (FAILED(hr))
	{
		Fatal("ID3DUserDefinedAnnotation failed: " + DXErrorToStr(hr));
		return false;
	}

#if RHI_VALIDATION_ENABLED
	ComPtr<ID3D11InfoQueue> d3dDebug;
	hr = device.As(&d3dDebug);
	if (SUCCEEDED(hr))
	{
		ComPtr<ID3D11InfoQueue> infoQueue;
		hr = d3dDebug.As(&infoQueue);
		if (SUCCEEDED(hr))
		{
			D3D11_MESSAGE_ID hide[] =
			{
				D3D11_MESSAGE_ID_DEVICE_DRAW_RESOURCE_FORMAT_SAMPLE_C_UNSUPPORTED,
				D3D11_MESSAGE_ID_QUERY_BEGIN_ABANDONING_PREVIOUS_RESULTS,
				D3D11_MESSAGE_ID_QUERY_END_ABANDONING_PREVIOUS_RESULTS
			};

			D3D11_INFO_QUEUE_FILTER filter{};
			filter.DenyList.NumIDs = static_cast<UINT>(std::size(hide));
			filter.DenyList.pIDList = hide;
			infoQueue->AddStorageFilterEntries(&filter);

			infoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, TRUE);
			infoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, TRUE);
			infoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_INFO, TRUE);
			infoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_MESSAGE, TRUE);
			infoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_WARNING, TRUE);
		}
	}
#endif

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.Width                 = data.width;
	swapChainDesc.Height                = data.height;
	swapChainDesc.Format                = ToD3D11(gContext.backBufferFormat);
	swapChainDesc.SampleDesc.Count      = 1;
	swapChainDesc.SampleDesc.Quality    = 0;
	swapChainDesc.BufferUsage           = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount           = RHI_BACKBUFFER_COUNT;
	//swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	DXGI_SWAP_CHAIN_FULLSCREEN_DESC fullScreenSwapChainDesc = {};
	fullScreenSwapChainDesc.Windowed = TRUE;

	ComPtr<IDXGISwapChain1> swapChain = nullptr;
	hr = dxgiFactory->CreateSwapChainForHwnd(device.Get(), data.hwnd, &swapChainDesc, &fullScreenSwapChainDesc, nullptr, swapChain.ReleaseAndGetAddressOf());
	if (FAILED(hr))
	{
		Fatal("CreateSwapChainForHwnd() failed: " + DXErrorToStr(hr));
		return false;
	}

	hr = dxgiFactory->MakeWindowAssociation(data.hwnd, DXGI_MWA_NO_ALT_ENTER);
	if (FAILED(hr))
	{
		Fatal("MakeWindowAssociation() failed: " + DXErrorToStr(hr));
		return false;
	}

	hr = swapChain.As(&gContext.swapChain);
	if (FAILED(hr))
	{
		Fatal("IDXGISwapChain as IDXGISwapChain4 failed: " + DXErrorToStr(hr));
		return false;
	}

	if (!CreateMainRenderTargetD3D11(data.width, data.height))
	{
		Fatal("CreateMainRenderTarget() failed");
		return false;
	}
	SetRenderTarget(nullptr, 0);

	return true;
}
//=============================================================================
void RHIBackend::DestroyAPI()
{
	gContext.Reset();
}
//=============================================================================
void RHIBackend::ResizeFrameBuffer(uint32_t width, uint32_t height)
{
	DestroyMainRenderTargetD3D11();
	HRESULT hr = gContext.swapChain->ResizeBuffers(RHI_BACKBUFFER_COUNT, (UINT)width, (UINT)height, ToD3D11(gContext.backBufferFormat), 0);
	if (FAILED(hr))
	{
		Fatal("ResizeBuffers() failed: " + DXErrorToStr(hr));
		return;
	}
	if (!CreateMainRenderTargetD3D11(width, height))
	{
		Fatal("CreateMainRenderTarget() failed");
		return;
	}
	SetRenderTarget(nullptr, 0);
	if (!gContext.viewport.has_value())
		gContext.viewportDirty = true;
}
//=============================================================================
void RHIBackend::Present()
{
	HRESULT hr = gContext.swapChain->Present(gContext.vsync ? 1 : 0, 0);
	if (FAILED(hr))
	{
		Fatal("Present() failed: " + DXErrorToStr(hr));
		return;
	}
}
//=============================================================================
void RHIBackend::Clear(const std::optional<glm::vec4>& color, const std::optional<float>& depth, const std::optional<uint8_t>& stencil)
{
	for (auto target : gContext.renderTargets)
	{
		if (color.has_value())
		{
			gContext.context->ClearRenderTargetView(target->GetD3D11RenderTargetView().Get(), (float*)&color.value());
		}

		if (depth.has_value() || stencil.has_value())
		{
			UINT flags = 0;
			if (depth.has_value()) flags |= D3D11_CLEAR_DEPTH;
			if (stencil.has_value()) flags |= D3D11_CLEAR_STENCIL;

			gContext.context->ClearDepthStencilView(target->GetD3D11DepthStencilView().Get(), flags, depth.value_or(1.0f), stencil.value_or(0));
		}
	}
}
//=============================================================================
void RHIBackend::Draw(uint32_t vertexCount, uint32_t vertexOffset, uint32_t instanceCount)
{
	EnsureGraphicsState();
	gContext.context->DrawInstanced(vertexCount, instanceCount, vertexOffset, 0);
}
//=============================================================================
void RHIBackend::DrawIndexed(uint32_t indexCount, uint32_t indexOffset, uint32_t instanceCount)
{
	EnsureGraphicsState();
	gContext.context->DrawIndexedInstanced(indexCount, instanceCount, indexOffset, 0, 0);
}
//=============================================================================
void RHIBackend::ReadPixels(const glm::i32vec2& pos, const glm::i32vec2& size, TextureHandle* dstTextureHandle)
{
	if (size.x <= 0 || size.y <= 0) return;

	auto dstTexture = (Texture2DD3D11*)dstTextureHandle;
	auto format = gContext.GetBackBufferFormat();

	assert(dstTexture->GetWidth() == size.x);
	assert(dstTexture->GetHeight() == size.y);
	assert(dstTexture->GetFormat() == format);

	auto target = gContext.renderTargets.at(0);

	ComPtr<ID3D11Resource> rtvResource;
	target->GetD3D11RenderTargetView()->GetResource(rtvResource.GetAddressOf());

	ComPtr<ID3D11Texture2D> rtvTexture;
	rtvResource.As(&rtvTexture);

	D3D11_TEXTURE2D_DESC desc = { 0 };
	rtvTexture->GetDesc(&desc);
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
		gContext.context->CopySubresourceRegion(dstTexture->GetD3D11Texture2D().Get(), 0, dst_x, dst_y, 0, rtvResource.Get(), 0, &box);
	}
}
//=============================================================================
ShaderHandle* RHIBackend::CreateShader(const std::string& vertexCode, const std::string& fragmentCode, const std::vector<std::string>& defines)
{
	auto shader = new ShaderD3D11(vertexCode, fragmentCode, defines);
	return (ShaderHandle*)shader;
}
//=============================================================================
void RHIBackend::DestroyShader(ShaderHandle* handle)
{
	if (handle)
	{
		auto shader = (ShaderD3D11*)handle;
		delete shader;
	}
}
//=============================================================================
TextureHandle* RHIBackend::CreateTexture(uint32_t width, uint32_t height, PixelFormat format, uint32_t mip_count)
{
	auto texture = new Texture2DD3D11(width, height, format, mip_count);
	return (TextureHandle*)texture;
}
//=============================================================================
void RHIBackend::WriteTexturePixels(TextureHandle* handle, uint32_t width, uint32_t height, PixelFormat format, const void* memory, uint32_t mip_level, uint32_t offset_x, uint32_t offset_y)
{
	auto texture = (Texture2DD3D11*)handle;
	texture->Write(width, height, format, memory, mip_level, offset_x, offset_y);
}
//=============================================================================
void RHIBackend::GenerateMips(TextureHandle* handle)
{
	auto texture = (Texture2DD3D11*)handle;
	texture->GenerateMips();
}
//=============================================================================
void RHIBackend::DestroyTexture(TextureHandle* handle)
{
	if (handle)
	{
		auto texture = (Texture2DD3D11*)handle;
		delete texture;
	}
}
//=============================================================================
RenderTargetHandle* RHIBackend::CreateRenderTarget(uint32_t width, uint32_t height, TextureHandle* textureHandle)
{
	auto texture = (Texture2DD3D11*)textureHandle;
	auto renderTarget = new RenderTargetD3D11(width, height, texture);
	return (RenderTargetHandle*)renderTarget;
}
//=============================================================================
void RHIBackend::DestroyRenderTarget(RenderTargetHandle* handle)
{
	if (handle)
	{
		auto render_target = (RenderTargetD3D11*)handle;
		delete render_target;
	}
}
//=============================================================================
VertexBufferHandle* RHIBackend::CreateVertexBuffer(size_t size, size_t stride)
{
	auto buffer = new VertexBufferD3D11(size, stride);
	return (VertexBufferHandle*)buffer;
}
//=============================================================================
void RHIBackend::DestroyVertexBuffer(VertexBufferHandle* handle)
{
	if (handle)
	{
		auto buffer = (VertexBufferD3D11*)handle;
		delete buffer;
	}
}
//=============================================================================
void RHIBackend::WriteVertexBufferMemory(VertexBufferHandle* handle, const void* memory, size_t size, size_t stride)
{
	auto buffer = (VertexBufferD3D11*)handle;
	buffer->Write(memory, size);
	buffer->SetStride(stride);
}
//=============================================================================
IndexBufferHandle* RHIBackend::CreateIndexBuffer(size_t size, size_t stride)
{
	auto buffer = new IndexBufferD3D11(size, stride);
	return (IndexBufferHandle*)buffer;
}
//=============================================================================
void RHIBackend::DestroyIndexBuffer(IndexBufferHandle* handle)
{
	if (handle)
	{
		auto buffer = (IndexBufferD3D11*)handle;
		delete buffer;
	}
}
//=============================================================================
void RHIBackend::WriteIndexBufferMemory(IndexBufferHandle* handle, const void* memory, size_t size, size_t stride)
{
	auto buffer = (IndexBufferD3D11*)handle;
	buffer->Write(memory, size);
	buffer->SetStride(stride);
}
//=============================================================================
UniformBufferHandle* RHIBackend::CreateUniformBuffer(size_t size)
{
	auto buffer = new UniformBufferD3D11(size);
	return (UniformBufferHandle*)buffer;
}
//=============================================================================
void RHIBackend::DestroyUniformBuffer(UniformBufferHandle* handle)
{
	if (handle)
	{
		auto buffer = (UniformBufferD3D11*)handle;
		delete buffer;
	}
}
//=============================================================================
void RHIBackend::WriteUniformBufferMemory(UniformBufferHandle* handle, const void* memory, size_t size)
{
	auto buffer = (UniformBufferD3D11*)handle;
	buffer->Write(memory, size);
}
//=============================================================================
void RHIBackend::SetTopology(Topology topology)
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
void RHIBackend::SetViewport(std::optional<Viewport> viewport)
{
	gContext.viewport = viewport;
	gContext.viewportDirty = true;
}
//=============================================================================
void RHIBackend::SetScissor(std::optional<Scissor> scissor)
{
	if (scissor.has_value())
	{
		auto& value = scissor.value();
		D3D11_RECT rect =
		{
			.left = static_cast<LONG>(value.position.x),
			.top = static_cast<LONG>(value.position.y),
			.right = static_cast<LONG>(value.position.x + value.size.x),
			.bottom = static_cast<LONG>(value.position.y + value.size.y)
		};
		gContext.context->RSSetScissorRects(1, &rect);
	}

	gContext.rasterizerState.scissorEnabled = scissor.has_value();
	gContext.rasterizerStateDirty = true;
}
//=============================================================================
void RHIBackend::SetBlendMode(const std::optional<BlendMode>& blend_mode)
{
	gContext.blendMode = blend_mode;
	gContext.blendModeDirty = true;
}
//=============================================================================
void RHIBackend::SetDepthMode(const std::optional<DepthMode>& depth_mode)
{
	gContext.depthStencilState.depthMode = depth_mode;
	gContext.depthStencilStateDirty = true;
}
//=============================================================================
void RHIBackend::SetStencilMode(const std::optional<StencilMode>& stencil_mode)
{
	gContext.depthStencilState.stencilMode = stencil_mode;
	gContext.depthStencilStateDirty = true;
}
//=============================================================================
void RHIBackend::SetCullMode(CullMode cull_mode)
{
	gContext.rasterizerState.cullMode = cull_mode;
	gContext.rasterizerStateDirty = true;
}
//=============================================================================
void RHIBackend::SetSampler(Filter value)
{
	gContext.samplerState.filter = value;
	gContext.samplerStateDirty = true;
}
//=============================================================================
void RHIBackend::SetTextureAddress(TextureAddress value)
{
	gContext.samplerState.textureAddress = value;
	gContext.samplerStateDirty = true;
}
//=============================================================================
void RHIBackend::SetFrontFace(FrontFace value)
{
	gContext.rasterizerState.frontFace = value;
	gContext.rasterizerStateDirty = true;
}
//=============================================================================
void RHIBackend::SetDepthBias(const std::optional<DepthBias> depth_bias)
{
	gContext.rasterizerState.depthBias = depth_bias;
	gContext.rasterizerStateDirty = true;
}
//=============================================================================
void RHIBackend::SetShader(ShaderHandle* handle)
{
	gContext.shader = (ShaderD3D11*)handle;
	gContext.shaderDirty = true;
	gContext.inputLayoutsDirty = true;
}
//=============================================================================
void RHIBackend::SetInputLayout(const std::vector<InputLayout>& value)
{
	gContext.inputLayouts = value;
	gContext.inputLayoutsDirty = true;
}
//=============================================================================
void RHIBackend::SetTexture(uint32_t binding, TextureHandle* handle)
{
	auto texture = (Texture2DD3D11*)handle;
	gContext.context->PSSetShaderResources((UINT)binding, 1, texture->GetD3D11ShaderResourceView().GetAddressOf());
	gContext.textures[binding] = texture;
}
//=============================================================================
void RHIBackend::SetRenderTarget(const RenderTarget** render_target, size_t count)
{
	if (count == 0)
	{
		gContext.context->OMSetRenderTargets(1, 
			gContext.mainRenderTarget->GetD3D11RenderTargetView().GetAddressOf(),
			gContext.mainRenderTarget->GetD3D11DepthStencilView().Get());

		gContext.renderTargets = { gContext.mainRenderTarget };

		if (!gContext.viewport.has_value())
			gContext.viewportDirty = true;
		return;
	}

	ComPtr<ID3D11ShaderResourceView> prev_shader_resource_view;
	gContext.context->PSGetShaderResources(0, 1, prev_shader_resource_view.GetAddressOf());

	std::vector<ID3D11RenderTargetView*> render_target_views;
	std::optional<ID3D11DepthStencilView*> depth_stencil_view;

	gContext.renderTargets.clear();

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

		gContext.renderTargets.push_back(target);
	}
	gContext.context->OMSetRenderTargets((UINT)render_target_views.size(), render_target_views.data(), depth_stencil_view.value_or(nullptr));

	if (!gContext.viewport.has_value())
		gContext.viewportDirty = true;
}
//=============================================================================
void RHIBackend::SetVertexBuffer(const VertexBuffer** vertex_buffer, size_t count)
{
	std::vector<ID3D11Buffer*> buffers(count);
	std::vector<UINT> strides(count);
	std::vector<UINT> offsets(count);

	for (size_t i = 0; i < count; i++)
	{
		auto buffer = (VertexBufferD3D11*)(VertexBufferHandle*)*(VertexBuffer*)vertex_buffer[i];
		buffers[i] = buffer->GetD3D11Buffer().Get();
		strides[i] = (UINT)buffer->GetStride();
		offsets[i] = 0;
	}
	gContext.context->IASetVertexBuffers(0, (UINT)buffers.size(), buffers.data(), strides.data(), offsets.data());
}
//=============================================================================
void RHIBackend::SetIndexBuffer(IndexBufferHandle* handle)
{
	auto buffer = (IndexBufferD3D11*)handle;
	auto stride = (UINT)buffer->GetStride();
	gContext.context->IASetIndexBuffer(buffer->GetD3D11Buffer().Get(), buffer->GetStride() == 2 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT, 0);
}
//=============================================================================
void RHIBackend::SetUniformBuffer(uint32_t binding, UniformBufferHandle* handle)
{
	auto buffer = (UniformBufferD3D11*)handle;
	gContext.context->VSSetConstantBuffers(binding, 1, buffer->GetD3D11Buffer().GetAddressOf());
	gContext.context->PSSetConstantBuffers(binding, 1, buffer->GetD3D11Buffer().GetAddressOf());
}
//=============================================================================
#endif // RENDER_D3D11