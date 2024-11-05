#pragma once

class WindowSystem;

struct RenderSystemCreateInfo final
{
};

class RenderSystem final
{
public:
	~RenderSystem();

	bool Create(const WindowSystem& window, const RenderSystemCreateInfo& createInfo);
	void Destroy();
private:
	ID3D11Device5*        m_device{ nullptr };
	ID3D11DeviceContext4* m_deviceContext{ nullptr };
	IDXGISwapChain4*      m_swapChain{ nullptr };
};