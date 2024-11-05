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
	assert(!m_device);
}

bool RenderSystem::Create(const WindowSystem& window, const RenderSystemCreateInfo& createInfo)
{
	assert(!thisRenderSystem);

	if (!createFactory()) return false;

	m_window = window.GetWindowHWND();
	m_outputSize.left = m_outputSize.top = 0;
	m_outputSize.right = static_cast<long>(window.GetWindowWidth());
	m_outputSize.bottom = static_cast<long>(window.GetWindowHeight());

	m_allowTearing = createInfo.AllowTearing;
	// Determines whether tearing support is available for fullscreen borderless windows.
	if (m_allowTearing)
	{
		BOOL allowTearing = FALSE;
		HRESULT hr = m_dxgiFactory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing));
		if (FAILED(hr) || !allowTearing)
		{
			m_allowTearing = false;
			Warning("Variable refresh rate displays not supported");
		}
	}

	// Create D3D11 Device and Context
	if (!createDevice()) return false;

	// Set up debug layer to break on D3D11 errors
	setDebugLayer();
	
	// Create Swap Chain
	if (!resizeSwapChain()) return false;

	thisRenderSystem = this;

	return true;
}

void RenderSystem::Destroy()
{
	if (m_renderTargetView) m_renderTargetView->Release();
	if (m_renderTarget) m_renderTarget->Release();
	if (m_depthStencilView) m_depthStencilView->Release();
	if (m_depthStencil) m_depthStencil->Release();

	if (m_swapChain) m_swapChain->Release();
	if (m_annotation) m_annotation->Release();
	if (m_deviceContext) m_deviceContext->Release();
	if (m_device) m_device->Release();
	if (m_dxgiFactory) m_dxgiFactory->Release();

	m_annotation = nullptr;
	m_dxgiFactory = nullptr;
	m_device = nullptr;
	m_deviceContext = nullptr;
	m_swapChain = nullptr;
	m_renderTargetView = nullptr;
	m_renderTarget = nullptr;
	m_depthStencilView = nullptr;
	m_depthStencil = nullptr;

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
		// Handle color space settings for HDR
		UpdateColorSpace();
		return false;
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

		if (!m_dxgiFactory->IsCurrent())
		{
			UpdateColorSpace();
		}
	}
}

bool RenderSystem::createFactory()
{
	UINT flags = 0;
#if defined(_DEBUG)
	flags = DXGI_CREATE_FACTORY_DEBUG;
#endif
	HRESULT hResult = CreateDXGIFactory2(flags, IID_PPV_ARGS(&m_dxgiFactory));
	if (FAILED(hResult))
	{
		Fatal("CreateDXGIFactory2() failed");
		return false;
	}

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

	return true;
}

// This method acquires the first available hardware adapter.
// If no such adapter can be found, *ppAdapter will be set to nullptr.
void RenderSystem::getHardwareAdapter(IDXGIAdapter4** ppAdapter)
{
	*ppAdapter = nullptr;

	IDXGIAdapter1* adapter;
	for (UINT adapterIndex = 0;
		SUCCEEDED(m_dxgiFactory->EnumAdapterByGpuPreference(
			adapterIndex,
			DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
			IID_PPV_ARGS(&adapter)));
		adapterIndex++)
	{
		DXGI_ADAPTER_DESC1 desc;
		if (FAILED(adapter->GetDesc1(&desc)))
		{
			Fatal("IDXGIAdapter1::GetDesc1() failed");
			return;
		}

		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
		{
			// Don't select the Basic Render Driver adapter.
			continue;
		}

		break;
	}

	if (!adapter)
	{
		for (UINT adapterIndex = 0;
			SUCCEEDED(m_dxgiFactory->EnumAdapters1(adapterIndex, &adapter));
				adapterIndex++)
		{
			DXGI_ADAPTER_DESC1 desc;
			if (FAILED(adapter->GetDesc1(&desc)))
			{
				Fatal("IDXGIAdapter1::GetDesc1() failed");
				return;
			}

			if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			{
				// Don't select the Basic Render Driver adapter.
				continue;
			}

			break;
		}
	}

	if (adapter)
	{
		adapter->QueryInterface(&*ppAdapter);
		adapter->Release();
	}
}

bool RenderSystem::createDevice()
{
	IDXGIAdapter4* adapter{ nullptr };
	getHardwareAdapter(&adapter);
	if (!adapter) return false;

	DXGI_ADAPTER_DESC adapterDesc;
	adapter->GetDesc(&adapterDesc);
	Print("Graphics Device: " + ToString(std::wstring(adapterDesc.Description)));

	D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_1, };
	UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#if defined(_DEBUG)
	creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	ID3D11Device* baseDevice;
	ID3D11DeviceContext* baseDeviceContext;
	HRESULT hResult = D3D11CreateDevice(adapter, D3D_DRIVER_TYPE_UNKNOWN, nullptr, creationFlags, featureLevels, ARRAYSIZE(featureLevels), D3D11_SDK_VERSION, &baseDevice, nullptr, &baseDeviceContext);
	adapter->Release();
	if (FAILED(hResult))
	{
		Fatal("D3D11CreateDevice() failed");
		return false;
	}

	hResult = baseDevice->QueryInterface(&m_device);
	baseDevice->Release();
	if (FAILED(hResult))
	{
		Fatal("QueryInterface ID3D11Device5 failed");
		return false;
	}

	hResult = baseDeviceContext->QueryInterface(&m_deviceContext);
	if (FAILED(hResult))
	{
		baseDeviceContext->Release();
		Fatal("QueryInterface ID3D11DeviceContext4 failed");
		return false;
	}
	hResult = baseDeviceContext->QueryInterface(&m_annotation);
	baseDeviceContext->Release();
	if (FAILED(hResult))
	{
		Fatal("QueryInterface ID3DUserDefinedAnnotation failed");
		return false;
	}

	return true;
}

void RenderSystem::setDebugLayer()
{
#if defined(_DEBUG)
	ID3D11Debug* d3dDebug = nullptr;
	m_device->QueryInterface(&d3dDebug);
	if (d3dDebug)
	{
		ID3D11InfoQueue* d3dInfoQueue = nullptr;
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

			d3dInfoQueue->Release();
		}
		d3dDebug->Release();
	}
#endif
}

bool RenderSystem::resizeSwapChain()
{
	if (!m_deviceContext || !m_device) return false;

	m_deviceContext->OMSetRenderTargets(0, nullptr, nullptr);
	if (m_renderTargetView) m_renderTargetView->Release();
	if (m_depthStencilView) m_depthStencilView->Release();
	if (m_renderTarget) m_renderTarget->Release();
	if (m_depthStencil) m_depthStencil->Release();
	m_deviceContext->Flush();

	// Determine the render target size in pixels.
	const UINT backBufferWidth = std::max<UINT>(static_cast<UINT>(m_outputSize.right - m_outputSize.left), 1u);
	const UINT backBufferHeight = std::max<UINT>(static_cast<UINT>(m_outputSize.bottom - m_outputSize.top), 1u);
	const DXGI_FORMAT backBufferFormat = (m_flipPresent || m_allowTearing || m_enableHDR) ? NoSRGB(m_backBufferFormat) : m_backBufferFormat; // TODO: ?

	if (m_swapChain)
	{
		// If the swap chain already exists, resize it.
		HRESULT hr = m_swapChain->ResizeBuffers(
			m_backBufferCount,
			backBufferWidth,
			backBufferHeight,
			backBufferFormat,
			(m_allowTearing) ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0u
		);

		if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
		{
#if defined(_DEBUG)
			char buff[64] = {};
			sprintf_s(buff, "Device Lost on ResizeBuffers: Reason code 0x%08X\n",
				static_cast<unsigned int>((hr == DXGI_ERROR_DEVICE_REMOVED) ? m_device->GetDeviceRemovedReason() : hr));
			Print(buff);
#endif
			// If the device was removed for any reason, a new device and swap chain will need to be created.
			handleDeviceLost();

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
		// Create a descriptor for the swap chain.
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
		swapChainDesc.Width = backBufferWidth;
		swapChainDesc.Height = backBufferHeight;
		swapChainDesc.Format = backBufferFormat;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = m_backBufferCount;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
		swapChainDesc.SwapEffect = (m_flipPresent || m_allowTearing || m_enableHDR) ? DXGI_SWAP_EFFECT_FLIP_DISCARD : DXGI_SWAP_EFFECT_DISCARD;
		swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
		swapChainDesc.Flags = (m_allowTearing) ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0u;

		DXGI_SWAP_CHAIN_FULLSCREEN_DESC fsSwapChainDesc = {};
		fsSwapChainDesc.Windowed = TRUE;

		// Create a SwapChain from a Win32 window.
		if (FAILED(m_dxgiFactory->CreateSwapChainForHwnd(m_device, m_window, &swapChainDesc, &fsSwapChainDesc, nullptr, &m_swapChain)))
		{
			Fatal("CreateSwapChainForHwnd failed");
			return false;
		}

		// This class does not support exclusive full-screen mode and prevents DXGI from responding to the ALT+ENTER shortcut
		if (FAILED(m_dxgiFactory->MakeWindowAssociation(m_window, DXGI_MWA_NO_ALT_ENTER)))
		{
			Fatal("MakeWindowAssociation failed");
			return false;
		}
	}

	// Handle color space settings for HDR
	UpdateColorSpace();

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

	if (m_depthBufferFormat != DXGI_FORMAT_UNKNOWN)
	{
		// Create a depth stencil view for use with 3D rendering if needed.
		CD3D11_TEXTURE2D_DESC depthStencilDesc(
			m_depthBufferFormat,
			backBufferWidth,
			backBufferHeight,
			1, // Use a single array entry.
			1, // Use a single mipmap level.
			D3D11_BIND_DEPTH_STENCIL
		);

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

void RenderSystem::handleDeviceLost()
{
	if (m_deviceNotify) m_deviceNotify->OnDeviceLost();

	if (m_depthStencilView) m_depthStencilView->Release();
	if (m_renderTargetView) m_renderTargetView->Release();
	if (m_renderTarget) m_renderTarget->Release();
	if (m_depthStencil) m_depthStencil->Release();
	if (m_swapChain) m_swapChain->Release();
	if (m_deviceContext) m_deviceContext->Release();
	if (m_annotation) m_annotation->Release();

#if defined(_DEBUG)
	{
		ID3D11Debug* d3dDebug;
		if (SUCCEEDED(m_device->QueryInterface(&d3dDebug)))
		{
			d3dDebug->ReportLiveDeviceObjects(D3D11_RLDO_SUMMARY);
		}
	}
#endif

	m_device->Release();
	m_dxgiFactory->Release();

	if (!createFactory())
	{
		// TODO: error
	}
	if (!createDevice())
	{
		// TODO: error
	}
	setDebugLayer();
	if (!resizeSwapChain())
	{
		// TODO: error
	}

	if (m_deviceNotify) m_deviceNotify->OnDeviceRestored();
}

// Sets the color space for the swap chain in order to handle HDR output.
void RenderSystem::UpdateColorSpace()
{
	if (!m_dxgiFactory) return;

	if (!m_dxgiFactory->IsCurrent())
	{
		// Output information is cached on the DXGI Factory. If it is stale we need to create a new factory.
		if (!createFactory())
		{
			// TODO: error
		}
	}

	DXGI_COLOR_SPACE_TYPE colorSpace = DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709;

	bool isDisplayHDR10 = false;

	if (m_swapChain)
	{
		// To detect HDR support, we will need to check the color space in the primary DXGI output associated with the app at this point in time (using window/display intersection).

		// Get the rectangle bounds of the app window.
		RECT windowBounds;
		if (!GetWindowRect(m_window, &windowBounds))
		{
			Fatal("GetWindowRect");
			return;
		}

		const long ax1 = windowBounds.left;
		const long ay1 = windowBounds.top;
		const long ax2 = windowBounds.right;
		const long ay2 = windowBounds.bottom;

		IDXGIOutput* bestOutput{ nullptr };
		long bestIntersectArea = -1;

		IDXGIAdapter* adapter;
		for (UINT adapterIndex = 0;
			SUCCEEDED(m_dxgiFactory->EnumAdapters(adapterIndex, &adapter));
			++adapterIndex)
		{
			IDXGIOutput* output;
			for (UINT outputIndex = 0;
				SUCCEEDED(adapter->EnumOutputs(outputIndex, &output));
				++outputIndex)
			{
				// Get the rectangle bounds of current output.
				DXGI_OUTPUT_DESC desc;
				if (FAILED(output->GetDesc(&desc)))
				{
					Fatal("GetDesc failed");
					return;
				}
				const auto& r = desc.DesktopCoordinates;

				// Compute the intersection
				const long intersectArea = computeIntersectionArea(ax1, ay1, ax2, ay2, r.left, r.top, r.right, r.bottom);
				if (intersectArea > bestIntersectArea)
				{
					if (bestOutput) bestOutput->Release();
					bestOutput = output;
					bestIntersectArea = intersectArea;
				}
			}
		}

		if (bestOutput)
		{
			IDXGIOutput6* output6;
			if (SUCCEEDED(bestOutput->QueryInterface(&output6)))
			{
				DXGI_OUTPUT_DESC1 desc;
				if (FAILED(output6->GetDesc1(&desc)))
				{
					Fatal("GetDesc1 failed");
					return;
				}

				if (desc.ColorSpace == DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020)
				{
					// Display output is HDR10.
					isDisplayHDR10 = true;
				}
			}
		}
	}

	if ((m_enableHDR) && isDisplayHDR10)
	{
		switch (m_backBufferFormat)
		{
		case DXGI_FORMAT_R10G10B10A2_UNORM:
			// The application creates the HDR10 signal.
			colorSpace = DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020;
			break;

		case DXGI_FORMAT_R16G16B16A16_FLOAT:
			// The system creates the HDR10 signal; application uses linear values.
			colorSpace = DXGI_COLOR_SPACE_RGB_FULL_G10_NONE_P709;
			break;

		default:
			break;
		}
	}

	m_colorSpace = colorSpace;

	IDXGISwapChain3* swapChain3;
	if (m_swapChain && SUCCEEDED(m_swapChain->QueryInterface(&swapChain3)))
	{
		UINT colorSpaceSupport = 0;
		if (SUCCEEDED(swapChain3->CheckColorSpaceSupport(colorSpace, &colorSpaceSupport))
			&& (colorSpaceSupport & DXGI_SWAP_CHAIN_COLOR_SPACE_SUPPORT_FLAG_PRESENT))
		{
			if (FAILED(swapChain3->SetColorSpace1(colorSpace)))
			{
				Fatal("SetColorSpace1 failed");
				return;
			}
		}
	}
}