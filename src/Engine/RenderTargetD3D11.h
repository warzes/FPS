﻿#pragma once

#if RENDER_D3D11

class Texture2DD3D11;

class RenderTargetD3D11 final
{
public:
	RenderTargetD3D11() = delete;
	RenderTargetD3D11(uint32_t width, uint32_t height, Texture2DD3D11* texture);

	const auto& GetD3D11RenderTargetView() const { return m_renderTargetView; }
	const auto& GetD3D11DepthStencilView() const { return m_depthStencilView; }
	auto        GetTexture() const { return m_texture; }

private:
	Texture2DD3D11*                  m_texture{ nullptr };
	ComPtr<ID3D11RenderTargetView> m_renderTargetView;
	ComPtr<ID3D11Texture2D>        m_depthStencilTexture;
	ComPtr<ID3D11DepthStencilView> m_depthStencilView;
};

#endif // RENDER_D3D11