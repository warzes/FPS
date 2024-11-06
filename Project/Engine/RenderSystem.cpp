#include "stdafx.h"
#include "RenderSystem.h"
#include "WindowSystem.h"
#include "Log.h"
#include "CoreFunc.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "DXGI.lib")
#pragma comment(lib, "D3DCompiler.lib")
#if defined(_DEBUG)
#	pragma comment(lib, "dxguid.lib")
#endif

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

inline long computeIntersectionArea(long ax1, long ay1, long ax2, long ay2, long bx1, long by1, long bx2, long by2) noexcept
{
	return std::max(0l, std::min(ax2, bx2) - std::max(ax1, bx1)) * std::max(0l, std::min(ay2, by2) - std::max(ay1, by1));
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
	m_outputSize.left = m_outputSize.top = 0;
	m_outputSize.right = static_cast<long>(window.GetWindowWidth());
	m_outputSize.bottom = static_cast<long>(window.GetWindowHeight());

	m_enableGraphicsAPIValidation = createInfo.EnableGraphicsAPIValidation;
#if defined(_DEBUG)
	m_enableGraphicsAPIValidation = true;
#endif

	m_allowTearing = createInfo.AllowTearing;
	m_enableHDR = createInfo.EnableHDR;
	m_flipPresent = createInfo.FlipPresent;

	if (!create()) return false;

	thisRenderSystem = this;
	return true;
}

void RenderSystem::Destroy()
{
	if (m_deviceContext) m_deviceContext->OMSetRenderTargets(0, nullptr, nullptr);
	m_renderTargetView.Reset();
	m_renderTarget.Reset();
	m_depthStencilView.Reset();
	m_depthStencil.Reset();

	m_swapChain.Reset();
	m_deviceContext.Reset();
	m_device.Reset();

	thisRenderSystem = nullptr;
}

bool RenderSystem::Resize(int width, int height)
{
	RECT newRc;
	newRc.left = newRc.top = 0;
	newRc.right = static_cast<long>(width);
	newRc.bottom = static_cast<long>(height);
	if (newRc.right == m_outputSize.right && newRc.bottom == m_outputSize.bottom)
	{
		return true;
	}

	m_outputSize = newRc;
	return resizeSwapChain();
}

void RenderSystem::Present()
{
	m_swapChain->Present(1, 0);

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
		handleDeviceLost();
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

bool RenderSystem::create()
{
	setupDebug();
	if (!selectAdapter()) return false;
	if (!createDevice()) return false;
	setDebugLayer();
	if (!resizeSwapChain()) return false;
	return true;
}

void RenderSystem::setupDebug()
{
#if defined(_DEBUG)
	ComPtr<IDXGIInfoQueue> dxgiInfoQueue;
	if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiInfoQueue))))
	{
		dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, true);
		dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, true);

		DXGI_INFO_QUEUE_MESSAGE_ID hide[] =
		{
			80 /* IDXGISwapChain::GetContainingOutput: The swapchain's adapter does not control the output on which the swapchain's window resides. */,
		};
		DXGI_INFO_QUEUE_FILTER filter = {};
		filter.DenyList.NumIDs = static_cast<UINT>(std::size(hide));
		filter.DenyList.pIDList = hide;
		dxgiInfoQueue->AddStorageFilterEntries(DXGI_DEBUG_DXGI, &filter);
	}
	else
	{
		Error("DXGIGetDebugInterface1 failed");
	}
#endif
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
	m_adapter = getHardwareAdapter(dxgiFactory);
	if (!m_adapter) return false;

	DXGI_ADAPTER_DESC adapterDesc;
	m_adapter->GetDesc(&adapterDesc);
	Print("Graphics Device: " + ToString(std::wstring(adapterDesc.Description)));

	return true;
}

ComPtr<IDXGIAdapter4> RenderSystem::getHardwareAdapter(ComPtr<IDXGIFactory6> factory)
{
	ComPtr<IDXGIAdapter4> adapter = nullptr;
	for (UINT adapterIndex = 0;
		SUCCEEDED(factory->EnumAdapterByGpuPreference(adapterIndex, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter)));
		adapterIndex++)
	{
		DXGI_ADAPTER_DESC1 desc;
		if (FAILED(adapter->GetDesc1(&desc)))
		{
			Fatal("IDXGIAdapter1::GetDesc1() failed");
			return nullptr;
		}

		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
		{
			// Don't select the Basic Render Driver adapter.
			continue;
		}

		break;
	}
	return adapter;
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

	return true;
}

void RenderSystem::setDebugLayer()
{
#if defined(_DEBUG)
	ComPtr<ID3D11Debug> d3dDebug = nullptr;
	m_device->QueryInterface(&d3dDebug);
	if (d3dDebug)
	{
		ComPtr<ID3D11InfoQueue> d3dInfoQueue = nullptr;
		if (SUCCEEDED(d3dDebug->QueryInterface(&d3dInfoQueue)))
		{
			d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
			d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);

			D3D11_MESSAGE_ID hide[] =
			{
				D3D11_MESSAGE_ID_SETPRIVATEDATA_CHANGINGPARAMS,
			};
			D3D11_INFO_QUEUE_FILTER filter = {};
			filter.DenyList.NumIDs = static_cast<UINT>(std::size(hide));
			filter.DenyList.pIDList = hide;
			d3dInfoQueue->AddStorageFilterEntries(&filter);
		}
	}
#endif
}

bool RenderSystem::resizeSwapChain()
{
	if (!m_deviceContext || !m_device) return false;

	m_deviceContext->OMSetRenderTargets(0, nullptr, nullptr);
	m_renderTargetView.Reset();
	m_renderTarget.Reset();
	m_depthStencilView.Reset();
	m_depthStencil.Reset();
	m_deviceContext->Flush();

	// Determine the render target size in pixels.
	const UINT backBufferWidth = std::max<UINT>(static_cast<UINT>(m_outputSize.right - m_outputSize.left), 1u);
	const UINT backBufferHeight = std::max<UINT>(static_cast<UINT>(m_outputSize.bottom - m_outputSize.top), 1u);
	const DXGI_FORMAT backBufferFormat = (m_flipPresent || m_allowTearing || m_enableHDR) ? NoSRGB(m_backBufferFormat) : m_backBufferFormat; // TODO: ?

	if (m_swapChain)
	{
		// If the swap chain already exists, resize it.
		HRESULT hr = m_swapChain->ResizeBuffers(m_backBufferCount, backBufferWidth, backBufferHeight, backBufferFormat, (m_allowTearing) ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0u);
		if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
		{
#if defined(_DEBUG)
			char buff[64] = {};
			sprintf_s(buff, "Device Lost on ResizeBuffers: Reason code 0x%08X\n",
				static_cast<unsigned int>((hr == DXGI_ERROR_DEVICE_REMOVED) ? m_device->GetDeviceRemovedReason() : hr));
			Print(buff);
#endif
			// If the device was removed for any reason, a new device and swap chain will need to be created.
			if (!handleDeviceLost()) return false;

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
		if (!createSwapChain(backBufferWidth, backBufferHeight, backBufferFormat)) return false;
	}

	// Color space
	{
		uint32_t colorSpaceSupport = 0;
		HRESULT hr = m_swapChain->CheckColorSpaceSupport(m_colorSpace, &colorSpaceSupport);

		if (!(colorSpaceSupport & DXGI_SWAP_CHAIN_COLOR_SPACE_SUPPORT_FLAG_PRESENT))
			hr = E_FAIL;

		if (SUCCEEDED(hr))
			hr = m_swapChain->SetColorSpace1(m_colorSpace);

		if (FAILED(hr))
			Warning("IDXGISwapChain::SetColorSpace1() failed!");
	}

	// Background color
	{
		DXGI_RGBA color = { 0.0f, 0.0f, 0.0f, 1.0f };
		if (FAILED(m_swapChain->SetBackgroundColor(&color)))
			Warning("IDXGISwapChain::SetBackgroundColor() failed!");
	}

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
		CD3D11_TEXTURE2D_DESC depthStencilDesc(m_depthBufferFormat, backBufferWidth, backBufferHeight, 1, 1, D3D11_BIND_DEPTH_STENCIL);
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
	m_screenViewport = { 0.0f, 0.0f, static_cast<float>(backBufferWidth), static_cast<float>(backBufferHeight), 0.f, 1.f };

	return true;
}

bool RenderSystem::createSwapChain(UINT backBufferWidth, UINT backBufferHeight, DXGI_FORMAT backBufferFormat)
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
	swapChainDesc.Width = backBufferWidth;
	swapChainDesc.Height = backBufferHeight;
	swapChainDesc.Format = backBufferFormat;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = m_backBufferCount;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.Scaling = DXGI_SCALING_NONE;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
	swapChainDesc.Flags = (m_allowTearing) ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0u;

	// Create a SwapChain from a Win32 window.
	if (FAILED(dxgiFactory6->CreateSwapChainForHwnd(m_device, m_windowHWND, &swapChainDesc, nullptr, nullptr, (IDXGISwapChain1**)&m_swapChain)))
	{
		Fatal("IDXGIFactory::CreateSwapChainForHwnd() failed");
		return false;
	}

	// This class does not support exclusive full-screen mode and prevents DXGI from responding to the ALT+ENTER shortcut
	if (FAILED(dxgiFactory6->MakeWindowAssociation(m_windowHWND, DXGI_MWA_NO_WINDOW_CHANGES | DXGI_MWA_NO_ALT_ENTER)))
	{
		Fatal("IDXGIFactory::MakeWindowAssociation() failed");
		return false;
	}

	return true;
}

bool RenderSystem::handleDeviceLost()
{
	if (m_deviceNotify) m_deviceNotify->OnDeviceLost();

	m_depthStencilView.Reset();
	m_renderTargetView.Reset();
	m_renderTarget.Reset();
	m_depthStencil.Reset();
	m_swapChain.Reset();
	m_deviceContext.Reset();
#if defined(_DEBUG)
	{
		ID3D11Debug* d3dDebug;
		if (SUCCEEDED(m_device->QueryInterface(&d3dDebug)))
		{
			d3dDebug->ReportLiveDeviceObjects(D3D11_RLDO_SUMMARY);
		}
	}
#endif
	m_device.Reset();

	if (!create()) return false;

	if (m_deviceNotify) m_deviceNotify->OnDeviceRestored();

	return true;
}