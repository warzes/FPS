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

	void UpdateColorSpace();

	RECT GetOutputSize() const noexcept { return m_outputSize; }
	ID3D11Device5* GetDevice() const noexcept { return m_device; }
	ID3D11DeviceContext4* GetDeviceContext() const noexcept { return m_deviceContext; }
	ID3D11RenderTargetView* GetRenderTargetView()const noexcept { return m_renderTargetView; }
	ID3D11DepthStencilView* GetDepthStencilView()const noexcept { return m_depthStencilView; }
private:
	bool createFactory();
	void getHardwareAdapter(IDXGIAdapter4** ppAdapter);
	bool createDevice();
	void setDebugLayer();
	bool resizeSwapChain();
	void handleDeviceLost();

	ComPtr - везде

	ComPtr<IDXGIFactory6>      m_dxgiFactory{ nullptr };
	ID3D11Device5*             m_device{ nullptr };
	ID3D11DeviceContext4*      m_deviceContext{ nullptr };
	ID3DUserDefinedAnnotation* m_annotation{ nullptr };
	IDeviceNotify*             m_deviceNotify{ nullptr };
	DXGI_COLOR_SPACE_TYPE      m_colorSpace{ DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709 };
	HWND                       m_window{ nullptr };
	DXGI_FORMAT                m_backBufferFormat{ DXGI_FORMAT_B8G8R8A8_UNORM_SRGB };
	DXGI_FORMAT                m_depthBufferFormat{ DXGI_FORMAT_D32_FLOAT };
	UINT                       m_backBufferCount{ 2 };
	IDXGISwapChain1*           m_swapChain{ nullptr };

	ID3D11Texture2D*           m_renderTarget{ nullptr };
	ID3D11RenderTargetView*    m_renderTargetView{ nullptr };
	ID3D11Texture2D*           m_depthStencil{ nullptr };
	ID3D11DepthStencilView*    m_depthStencilView{ nullptr };
	D3D11_VIEWPORT             m_screenViewport{};
	RECT                       m_outputSize{};

	bool                       m_allowTearing{ false };
	bool                       m_enableHDR{ false };
	bool                       m_flipPresent{ false };
};