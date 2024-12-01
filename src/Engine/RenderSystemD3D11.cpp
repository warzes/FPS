#include "stdafx.h"
#if RENDER_D3D11
#include "RenderSystem.h"
#include "ContextD3D11.h"
#include "TextureD3D11.h"
#include "RenderTargetD3D11.h"
#include "BufferD3D11.h"
#include "ShaderD3D11.h"
#include "Log.h"
//=============================================================================
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")
//=============================================================================
RenderContext gContext{};
//=============================================================================
bool RenderSystem::createAPI(const WindowData& data, const RenderSystemCreateInfo& createInfo)
{
	gContext.vsync = createInfo.vsync;

	ComPtr<IDXGIFactory6> dxgiFactory;
	if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(dxgiFactory.GetAddressOf()))))
	{
		Fatal("CreateDXGIFactory1() failed");
		return false;
	}

	const auto gpuPreference = DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE;
	if (FAILED(dxgiFactory->EnumAdapterByGpuPreference(0, gpuPreference, IID_PPV_ARGS(gContext.adapter.GetAddressOf()))))
	{
		Fatal("EnumAdapterByGpuPreference() failed");
		return false;
	}

	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
	swapChainDesc.BufferCount = 2;
	swapChainDesc.BufferDesc.Width = data.width;
	swapChainDesc.BufferDesc.Height = data.height;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // TODO: DXGI_FORMAT_B8G8R8A8_UNORM?
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.OutputWindow = data.hwnd;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.Windowed = TRUE;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

#if defined(_DEBUG)
	UINT flags = D3D11_CREATE_DEVICE_DEBUG;
#else
	UINT flags = 0;
#endif

	if (FAILED(D3D11CreateDeviceAndSwapChain(gContext.adapter.Get(), D3D_DRIVER_TYPE_UNKNOWN, nullptr, flags, nullptr, 0,
		D3D11_SDK_VERSION, &swapChainDesc, gContext.swapChain.GetAddressOf(), gContext.device.GetAddressOf(),
		nullptr, gContext.context.GetAddressOf())))
	{
		Fatal("D3D11CreateDeviceAndSwapChain() failed");
		return false;
	}

#if defined(_DEBUG)
	ComPtr<ID3D11InfoQueue> infoQueue;
	if (FAILED(gContext.device->QueryInterface(IID_PPV_ARGS(infoQueue.GetAddressOf()))))
	{
		Fatal("QueryInterface() failed");
		return false;
	}

	infoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
	infoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
	infoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_INFO, true);
	infoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_MESSAGE, true);
	infoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_WARNING, true);
#endif

	if (!CreateMainRenderTarget(data.width, data.height))
	{
		Fatal("CreateMainRenderTarget() failed");
		return false;
	}
	SetRenderTarget(std::nullopt);

	return true;
}
//=============================================================================
void RenderSystem::destroyAPI()
{
	DestroyMainRenderTarget();
	// TODO: очистить gContext
	gContext.swapChain.Reset();
	gContext.context.Reset();
	gContext.device.Reset();
	gContext.adapter.Reset();
}
//=============================================================================
void RenderSystem::resize(uint32_t width, uint32_t height)
{
	DestroyMainRenderTarget();
	if (FAILED(gContext.swapChain->ResizeBuffers(0, (UINT)width, (UINT)height, DXGI_FORMAT_R8G8B8A8_UNORM, 0)))
	{
		Fatal("ResizeBuffers() failed");
		return;
	}
	if (!CreateMainRenderTarget(width, height))
	{
		Fatal("CreateMainRenderTarget() failed");
		return;
	}
	SetRenderTarget(std::nullopt);
}
//=============================================================================
void RenderSystem::present()
{
	if (FAILED(gContext.swapChain->Present(gContext.vsync ? 1 : 0, 0)))
	{
		Fatal("Present() failed");
		return;
	}
}
//=============================================================================
void RenderSystem::SetRenderTarget(std::nullopt_t)
{
	gContext.context->OMSetRenderTargets(1, gContext.mainRenderTarget->GetD3D11RenderTargetView().GetAddressOf(), gContext.mainRenderTarget->GetD3D11DepthStencilView().Get());

	gContext.renderTargets = { gContext.mainRenderTarget };

	if (!gContext.viewport.has_value())
		gContext.viewport_dirty = true;
}
//=============================================================================
#endif // RENDER_D3D11