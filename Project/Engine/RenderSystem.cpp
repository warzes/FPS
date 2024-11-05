#include "stdafx.h"
#include "RenderSystem.h"
#include "WindowSystem.h"
#include "Log.h"

RenderSystem::~RenderSystem()
{
}

bool RenderSystem::Create(const WindowSystem& window, const RenderSystemCreateInfo& createInfo)
{
	// Create D3D11 Device and Context
	{
		ID3D11Device* baseDevice;
		ID3D11DeviceContext* baseDeviceContext;
		D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_1 };
		UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
		#if defined(_DEBUG)
		creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
		#endif

		HRESULT hResult = D3D11CreateDevice(0, D3D_DRIVER_TYPE_HARDWARE, 0, creationFlags, featureLevels, ARRAYSIZE(featureLevels), D3D11_SDK_VERSION, &baseDevice, 0, &baseDeviceContext);
		if (FAILED(hResult))
		{
			Fatal("D3D11CreateDevice() failed");
			return false;
		}

		hResult = baseDevice->QueryInterface(__uuidof(ID3D11Device5), (void**)&m_device);
		if (FAILED(hResult))
		{
			Fatal("QueryInterface ID3D11Device5 failed");
			return false;
		}
		baseDevice->Release();

		hResult = baseDeviceContext->QueryInterface(__uuidof(ID3D11DeviceContext4), (void**)&m_deviceContext);
		if (FAILED(hResult))
		{
			Fatal("QueryInterface ID3D11DeviceContext4 failed");
			return false;
		}
		baseDeviceContext->Release();
	}
	
	// Set up debug layer to break on D3D11 errors
#if defined(_DEBUG)
	ID3D11Debug* d3dDebug = nullptr;
	m_device->QueryInterface(__uuidof(ID3D11Debug), (void**)&d3dDebug);
	if (d3dDebug)
	{
		ID3D11InfoQueue* d3dInfoQueue = nullptr;
		if (SUCCEEDED(d3dDebug->QueryInterface(__uuidof(ID3D11InfoQueue), (void**)&d3dInfoQueue)))
		{
			d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
			d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
			d3dInfoQueue->Release();
		}
		d3dDebug->Release();
	}
#endif

	// Create Swap Chain
	{
		// Get DXGI Factory (needed to create Swap Chain)
		IDXGIFactory5* dxgiFactory{ nullptr };
		{
			IDXGIDevice4* dxgiDevice;
			HRESULT hResult = m_device->QueryInterface(__uuidof(IDXGIDevice4), (void**)&dxgiDevice);
			assert(SUCCEEDED(hResult));

			IDXGIAdapter* dxgiAdapter;
			hResult = dxgiDevice->GetAdapter(&dxgiAdapter);
			assert(SUCCEEDED(hResult));
			dxgiDevice->Release();

			DXGI_ADAPTER_DESC adapterDesc;
			dxgiAdapter->GetDesc(&adapterDesc);

			OutputDebugStringA("Graphics Device: ");
			OutputDebugStringW(adapterDesc.Description);

			hResult = dxgiAdapter->GetParent(__uuidof(IDXGIFactory5), (void**)&dxgiFactory);
			assert(SUCCEEDED(hResult));
			dxgiAdapter->Release();
		}

		DXGI_SWAP_CHAIN_DESC1 d3d11SwapChainDesc = {};
		d3d11SwapChainDesc.Width = 0; // use window width
		d3d11SwapChainDesc.Height = 0; // use window height
		d3d11SwapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
		d3d11SwapChainDesc.SampleDesc.Count = 1;
		d3d11SwapChainDesc.SampleDesc.Quality = 0;
		d3d11SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		d3d11SwapChainDesc.BufferCount = 2;
		d3d11SwapChainDesc.Scaling = DXGI_SCALING_STRETCH;
		d3d11SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		d3d11SwapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
		d3d11SwapChainDesc.Flags = 0;

		HRESULT hResult = dxgiFactory->CreateSwapChainForHwnd(m_device, hwnd, &d3d11SwapChainDesc, 0, 0, &m_swapChain);
		assert(SUCCEEDED(hResult));

		dxgiFactory->Release();
	}
https://github.com/kevinmoran/BeginnerDirect3D11/blob/master/01.%20Initialising%20Direct3D%2011/main.cpp
https://gist.github.com/d7samurai/abab8a580d0298cb2f34a44eec41d39d
http://www.directxtutorial.com/LessonList.aspx?listid=11

https://www.3dgep.com/learning-directx-12-1/

https://www.3dgep.com/introduction-to-directx-11/
https://www.3dgep.com/texturing-lighting-directx-11/
https://www.3dgep.com/forward-plus/

	return true;
}

void RenderSystem::Destroy()
{
}
