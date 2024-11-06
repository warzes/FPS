#pragma once

#include "ComPtr.h"

class WindowSystem;

struct IDeviceNotify
{
	virtual void OnDeviceLost() = 0;
	virtual void OnDeviceRestored() = 0;

protected:
	~IDeviceNotify() = default;
};

struct RenderSystemCreateInfo final
{
	bool EnableGraphicsAPIValidation{ false };
	bool AllowTearing{ false };
	bool EnableHDR{ false };
	bool FlipPresent{ true };
};

class RenderSystem final
{
public:
	~RenderSystem();

	bool Create(const WindowSystem& window, const RenderSystemCreateInfo& createInfo);
	void Destroy();

	bool Resize(int width, int height);
	void Present();

	RECT GetOutputSize() const noexcept { return m_outputSize; }
	IDXGIAdapter* GetAdapter() const noexcept { return m_adapter; }
	ID3D11Device5* GetDevice() const noexcept { return m_device; }
	ID3D11DeviceContext4* GetImmediateContext() const noexcept { return m_deviceContext; }
	ID3D11RenderTargetView* GetRenderTargetView()const noexcept { return m_renderTargetView; }
	ID3D11DepthStencilView* GetDepthStencilView()const noexcept { return m_depthStencilView; }
private:
	bool create();
	void setupDebug();
	bool selectAdapter();
	ComPtr<IDXGIAdapter4> getHardwareAdapter(ComPtr<IDXGIFactory6> factory);
	bool createDevice();
	void setDebugLayer();
	bool resizeSwapChain();
	bool createSwapChain(UINT backBufferWidth, UINT backBufferHeight, DXGI_FORMAT backBufferFormat);
	bool handleDeviceLost();

	HWND                           m_windowHWND{ nullptr };
	RECT                           m_outputSize{};

	ComPtr<IDXGIAdapter4>          m_adapter{ nullptr };
	ComPtr<ID3D11Device5>          m_device{ nullptr };
	ComPtr<ID3D11DeviceContext4>   m_deviceContext{ nullptr };	
	IDeviceNotify*                 m_deviceNotify{ nullptr };
	DXGI_COLOR_SPACE_TYPE          m_colorSpace{ DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709 };

	DXGI_FORMAT                    m_backBufferFormat{ DXGI_FORMAT_B8G8R8A8_UNORM_SRGB };
	DXGI_FORMAT                    m_depthBufferFormat{ DXGI_FORMAT_D32_FLOAT };
	UINT                           m_backBufferCount{ 2 };
	
	ComPtr<IDXGISwapChain4>        m_swapChain{ nullptr };

	ComPtr<ID3D11Texture2D>        m_renderTarget{ nullptr };
	ComPtr<ID3D11RenderTargetView> m_renderTargetView{ nullptr };
	ComPtr<ID3D11Texture2D>        m_depthStencil{ nullptr };
	ComPtr<ID3D11DepthStencilView> m_depthStencilView{ nullptr };
	D3D11_VIEWPORT                 m_screenViewport{};

	bool                           m_enableGraphicsAPIValidation{ false };
	bool                           m_allowTearing{ false };
	bool                           m_enableHDR{ false };
	bool                           m_flipPresent{ false };
};