#pragma once

#include "ComPtr.h"

class WindowSystem;

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

	bool Resize(uint32_t width, uint32_t height);
	void Present();

	IDXGIAdapter*           GetAdapter() const noexcept { return m_adapter; }
	ID3D11Device5*          GetDevice() const noexcept { return m_device; }
	ID3D11DeviceContext4*   GetImmediateContext() const noexcept { return m_deviceContext; }
	ID3D11RenderTargetView* GetRenderTargetView() const noexcept { return m_renderTargetView; }
	ID3D11DepthStencilView* GetDepthStencilView() const noexcept { return m_depthStencilView; }
private:
	bool selectAdapter();
	bool createDevice();
	bool resizeSwapChain();
	bool createSwapChain();
	bool createMainRenderTarget();
	void destroyMainRenderTarget();

	HWND                           m_windowHWND{ nullptr };
	//RECT                           m_outputSize{};

	ComPtr<IDXGIAdapter4>          m_adapter{ nullptr };
	ComPtr<ID3D11Device5>          m_device{ nullptr };
	ComPtr<ID3D11DeviceContext4>   m_deviceContext{ nullptr };

	DXGI_COLOR_SPACE_TYPE          m_colorSpace{ DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709 };
	UINT                           m_backBufferCount{ 2 };
	DXGI_FORMAT                    m_backBufferFormat{ DXGI_FORMAT_B8G8R8A8_UNORM_SRGB };
	DXGI_FORMAT                    m_depthBufferFormat{ DXGI_FORMAT_D32_FLOAT };
	ComPtr<IDXGISwapChain4>        m_swapChain{ nullptr };
	UINT                           m_backBufferWidth{ 0 };
	UINT                           m_backBufferHeight{ 0 };

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