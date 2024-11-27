#pragma once

#if RENDER_D3D11

#include "TextureD3D11.h"

class RenderTargetD3D11
{
public:
	RenderTargetD3D11(uint32_t width, uint32_t height, TextureD3D11* texture);

	const auto& GetD3D11RenderTargetView() const { return mRenderTargetView; }
	const auto& GetD3D11DepthStencilView() const { return mDepthStencilView; }
	auto GetTexture() const { return mTexture; }

private:
	ComPtr<ID3D11Texture2D> mDepthStencilTexture;
	ComPtr<ID3D11RenderTargetView> mRenderTargetView;
	ComPtr<ID3D11DepthStencilView> mDepthStencilView;
	TextureD3D11* mTexture = nullptr;
};

#endif // RENDER_D3D11