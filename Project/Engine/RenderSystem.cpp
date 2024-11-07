#include "stdafx.h"
#include "RenderSystem.h"
#include "WindowSystem.h"
#include "Log.h"
#include "CoreFunc.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "DXGI.lib")
#pragma comment(lib, "D3DCompiler.lib")

inline DXGI_FORMAT NoSRGB(DXGI_FORMAT fmt) noexcept
{
	switch (fmt)
	{
	case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:   return DXGI_FORMAT_R8G8B8A8_UNORM;
	case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:   return DXGI_FORMAT_B8G8R8A8_UNORM;
	case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:   return DXGI_FORMAT_B8G8R8X8_UNORM;
	default:                                return fmt;
	}
}

RenderSystem* thisRenderSystem = nullptr;

RenderSystem::~RenderSystem()
{
	assert(!thisRenderSystem);
}

bool RenderSystem::Create(const WindowSystem& window, const RenderSystemCreateInfo& createInfo)
{
	assert(!thisRenderSystem);

	m_windowHWND = window.GetWindowHWND();
	m_backBufferWidth = std::max(window.GetWindowWidth(), 1u);
	m_backBufferHeight = std::max(window.GetWindowHeight(), 1u);

	m_enableGraphicsAPIValidation = createInfo.EnableGraphicsAPIValidation;
#if defined(_DEBUG)
	m_enableGraphicsAPIValidation = true;
#endif

	m_allowTearing = createInfo.AllowTearing;
	m_enableHDR = createInfo.EnableHDR;
	m_flipPresent = createInfo.FlipPresent;

	if (!selectAdapter()) return false;
	if (!createDevice()) return false;
	if (!resizeSwapChain()) return false;

	thisRenderSystem = this;
	return true;
}

void RenderSystem::Destroy()
{
	destroyMainRenderTarget();

	m_swapChain.Reset();
	m_deviceContext.Reset();
	m_adapter.Reset();
	m_device.Reset();

	thisRenderSystem = nullptr;
}

bool RenderSystem::Resize(uint32_t width, uint32_t height)
{
	if (m_backBufferWidth == width && m_backBufferHeight == height)
		return true;

	m_backBufferWidth = std::max(width, 1u);
	m_backBufferHeight = std::max(height, 1u);
	return resizeSwapChain();
}

void RenderSystem::Present()
{
	HRESULT hr = E_FAIL;
	if (m_allowTearing)
	{
		// Recommended to always use tearing if supported when using a sync interval of 0.
		hr = m_swapChain->Present(0, DXGI_PRESENT_ALLOW_TEARING);
	}
	else
	{
		// The first argument instructs DXGI to block until VSync, putting the application to sleep until the next VSync. This ensures we don't waste any cycles rendering frames that will never be displayed to the screen.
		hr = m_swapChain->Present(1, 0);
	}

	// Discard the contents of the render target.
	// This is a valid operation only when the existing contents will be entirely overwritten. If dirty or scroll rects are used, this call should be removed.
	m_deviceContext->DiscardView(m_renderTargetView);

	if (m_depthStencilView)
	{
		// Discard the contents of the depth stencil.
		m_deviceContext->DiscardView(m_depthStencilView);
	}

	// If the device was removed either by a disconnection or a driver upgrade, we must recreate all device resources.
	if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
	{
#if defined(_DEBUG)
		char buff[64] = {};
		sprintf_s(buff, "Device Lost on Present: Reason code 0x%08X\n",
			static_cast<unsigned int>((hr == DXGI_ERROR_DEVICE_REMOVED) ? m_device->GetDeviceRemovedReason() : hr));
		Print(buff);
#endif
		//handleDeviceLost();
	}
	else
	{
		if (FAILED(hr))
		{
			Fatal("Render Present failed");
			return;
		}
	}
}

bool RenderSystem::selectAdapter()
{
	// create factory
	ComPtr<IDXGIFactory6> dxgiFactory{ nullptr };
	if (FAILED(CreateDXGIFactory2(m_enableGraphicsAPIValidation ? DXGI_CREATE_FACTORY_DEBUG : 0, IID_PPV_ARGS(&dxgiFactory))))
	{
		Fatal("CreateDXGIFactory2() failed");
		return false;
	}

	// Determines whether tearing support is available for fullscreen borderless windows.
	if (m_allowTearing)
	{
		BOOL allowTearing = FALSE;
		HRESULT hr = dxgiFactory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing));
		if (FAILED(hr) || !allowTearing)
		{
			m_allowTearing = false;
			Warning("Variable refresh rate displays not supported");
		}
	}

	// select adapter
	ComPtr<IDXGIAdapter4> adapter = nullptr;
	for (UINT adapterIndex = 0;
		SUCCEEDED(dxgiFactory->EnumAdapterByGpuPreference(adapterIndex, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter)));
		adapterIndex++)
	{
		DXGI_ADAPTER_DESC3 desc;
		if (FAILED(adapter->GetDesc3(&desc)))
		{
			Fatal("IDXGIAdapter::GetDesc3() failed");
			return false;
		}

		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) continue; // Don't select the Basic Render Driver adapter.

		break;
	}
	m_adapter = adapter;
	if (!m_adapter) return false;

	DXGI_ADAPTER_DESC3 adapterDesc;
	if (FAILED(m_adapter->GetDesc3(&adapterDesc)))
	{
		Fatal("IDXGIAdapter::GetDesc3() failed");
		return false;
	}
	Print("Graphics Device: " + ToString(std::wstring(adapterDesc.Description)));

	m_backBufferFormat = (m_flipPresent || m_allowTearing || m_enableHDR) ? NoSRGB(m_backBufferFormat) : m_backBufferFormat;

	return true;
}

bool RenderSystem::createDevice()
{
	D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_1, };
	UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
	if (m_enableGraphicsAPIValidation) creationFlags |= D3D11_CREATE_DEVICE_DEBUG;

	ComPtr<ID3D11Device> baseDevice;
	ComPtr<ID3D11DeviceContext> baseDeviceContext;
	if (FAILED(D3D11CreateDevice(m_adapter, D3D_DRIVER_TYPE_UNKNOWN, nullptr, creationFlags, featureLevels, ARRAYSIZE(featureLevels), D3D11_SDK_VERSION, &baseDevice, nullptr, &baseDeviceContext)))
	{
		Fatal("D3D11CreateDevice() failed");
		return false;
	}
	if (FAILED(baseDevice->QueryInterface(&m_device)))
	{
		Fatal("QueryInterface ID3D11Device5 failed");
		return false;
	}
	if (FAILED(baseDeviceContext->QueryInterface(&m_deviceContext)))
	{
		Fatal("QueryInterface ID3D11DeviceContext4 failed");
		return false;
	}

	if (m_enableGraphicsAPIValidation)
	{
		//ComPtr<ID3D11Debug> debug;
		//m_device->QueryInterface(&debug);
		//debug->ReportLiveDeviceObjects(D3D11_RLDO_SUMMARY | D3D11_RLDO_DETAIL);

		ComPtr<ID3D11InfoQueue> infoQueue;
		m_device->QueryInterface(&infoQueue);
		if (infoQueue)
		{
			infoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
			infoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
			infoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_INFO, true);
			infoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_MESSAGE, true);
			infoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_WARNING, true);
		}
	}

	return true;
}

bool RenderSystem::resizeSwapChain()
{
	destroyMainRenderTarget();

	if (m_swapChain)
	{
		// If the swap chain already exists, resize it.
		HRESULT hr = m_swapChain->ResizeBuffers(m_backBufferCount, m_backBufferWidth, m_backBufferHeight, m_backBufferFormat, (m_allowTearing) ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0u);
		if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
		{
#if defined(_DEBUG)
			char buff[64] = {};
			sprintf_s(buff, "Device Lost on ResizeBuffers: Reason code 0x%08X\n",
				static_cast<unsigned int>((hr == DXGI_ERROR_DEVICE_REMOVED) ? m_device->GetDeviceRemovedReason() : hr));
			Print(buff);
#endif
			// If the device was removed for any reason, a new device and swap chain will need to be created.
			//if (!handleDeviceLost()) return false;

			// Everything is set up now. Do not continue execution of this method. HandleDeviceLost will reenter this method and correctly set up the new device.
			return true;
		}
		else
		{
			if (FAILED(hr))
			{
				Fatal("swapchain resize failed");
				return false;
			}
		}
	}
	else
	{
		if (!createSwapChain()) return false;
	}

	if (!createMainRenderTarget()) return false;

	return true;
}

bool RenderSystem::createSwapChain()
{
	assert(m_adapter);
	ComPtr<IDXGIFactory6> dxgiFactory6;
	if (FAILED(m_adapter->GetParent(IID_PPV_ARGS(&dxgiFactory6))))
	{
		Fatal("IDXGIAdapter::GetParent() failed");
		return false;
	}

	// Create a descriptor for the swap chain.
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.Width = m_backBufferWidth;
	swapChainDesc.Height = m_backBufferHeight;
	swapChainDesc.Format = m_backBufferFormat;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = m_backBufferCount;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.Scaling = DXGI_SCALING_NONE;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
	swapChainDesc.Flags = (m_allowTearing) ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0u;

	DXGI_SWAP_CHAIN_FULLSCREEN_DESC swapChainFullscreenDesc = {};
	swapChainFullscreenDesc.Windowed = TRUE;

	// Create a SwapChain from a Win32 window.
	if (FAILED(dxgiFactory6->CreateSwapChainForHwnd(m_device, m_windowHWND, &swapChainDesc, &swapChainFullscreenDesc, nullptr, (IDXGISwapChain1**)&m_swapChain)))
	{
		Fatal("IDXGIFactory6::CreateSwapChainForHwnd() failed");
		return false;
	}

	// This class does not support exclusive full-screen mode and prevents DXGI from responding to the ALT+ENTER shortcut
	if (FAILED(dxgiFactory6->MakeWindowAssociation(m_windowHWND, DXGI_MWA_NO_WINDOW_CHANGES | DXGI_MWA_NO_ALT_ENTER)))
	{
		Fatal("IDXGIFactory6::MakeWindowAssociation() failed");
		return false;
	}

	// Color space
	{
		uint32_t colorSpaceSupport = 0;
		HRESULT hr = m_swapChain->CheckColorSpaceSupport(m_colorSpace, &colorSpaceSupport);
		if (!(colorSpaceSupport & DXGI_SWAP_CHAIN_COLOR_SPACE_SUPPORT_FLAG_PRESENT)) hr = E_FAIL;
		if (SUCCEEDED(hr)) hr = m_swapChain->SetColorSpace1(m_colorSpace);
		if (FAILED(hr)) Warning("IDXGISwapChain::SetColorSpace1() failed!");
	}

	// Background color
	{
		DXGI_RGBA color = { 0.0f, 0.0f, 0.0f, 1.0f };
		if (FAILED(m_swapChain->SetBackgroundColor(&color)))
			Warning("IDXGISwapChain::SetBackgroundColor() failed!");
	}

	return true;
}

bool RenderSystem::createMainRenderTarget()
{
	// Create a render target view of the swap chain back buffer.
	if (FAILED(m_swapChain->GetBuffer(0, IID_PPV_ARGS(&m_renderTarget))))
	{
		Fatal("Create a render target texture failed");
		return false;
	}

	CD3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc(D3D11_RTV_DIMENSION_TEXTURE2D, m_backBufferFormat);
	if (FAILED(m_device->CreateRenderTargetView(m_renderTarget, &renderTargetViewDesc, &m_renderTargetView)))
	{
		Fatal("Create a render target view failed");
		return false;
	}

	// Create a depth stencil view for use with 3D rendering if needed.
	if (m_depthBufferFormat != DXGI_FORMAT_UNKNOWN)
	{
		CD3D11_TEXTURE2D_DESC depthStencilDesc(m_depthBufferFormat, m_backBufferWidth, m_backBufferHeight, 1, 1, D3D11_BIND_DEPTH_STENCIL);
		if (FAILED(m_device->CreateTexture2D(&depthStencilDesc, nullptr, &m_depthStencil)))
		{
			Fatal("Create a depth target texture failed");
			return false;
		}
		if (FAILED(m_device->CreateDepthStencilView(m_depthStencil, nullptr, &m_depthStencilView)))
		{
			Fatal("Create a depth target view failed");
			return false;
		}
	}

	// Set the 3D rendering viewport to target the entire window.
	m_screenViewport = { 0.0f, 0.0f, static_cast<float>(m_backBufferWidth), static_cast<float>(m_backBufferHeight), 0.f, 1.f };

	return true;
}

void RenderSystem::destroyMainRenderTarget()
{
	if (m_deviceContext) m_deviceContext->OMSetRenderTargets(0, nullptr, nullptr);
	m_renderTargetView.Reset();
	m_renderTarget.Reset();
	m_depthStencilView.Reset();
	m_depthStencil.Reset();
	if (m_deviceContext) m_deviceContext->Flush();
}