#include "stdafx.h"
#if RENDER_D3D11
#include "RenderTargetD3D11.h"
#include "ContextD3D11.h"
//=============================================================================
RenderTargetD3D11::RenderTargetD3D11(uint32_t width, uint32_t height, TextureD3D11* texture) 
	: mTexture(texture)
{
	auto format = PixelFormatMap.at(texture->GetFormat());
	auto rtv_desc = CD3D11_RENDER_TARGET_VIEW_DESC(D3D11_RTV_DIMENSION_TEXTURE2D, format);
	gContext.device->CreateRenderTargetView(texture->GetD3D11Texture2D().Get(), &rtv_desc, mRenderTargetView.GetAddressOf());

	auto tex_desc = CD3D11_TEXTURE2D_DESC(DXGI_FORMAT_D24_UNORM_S8_UINT, width, height, 1, 1, D3D11_BIND_DEPTH_STENCIL);
	gContext.device->CreateTexture2D(&tex_desc, NULL, mDepthStencilTexture.GetAddressOf());

	auto dsv_desc = CD3D11_DEPTH_STENCIL_VIEW_DESC(D3D11_DSV_DIMENSION_TEXTURE2D, tex_desc.Format);
	gContext.device->CreateDepthStencilView(mDepthStencilTexture.Get(), &dsv_desc, mDepthStencilView.GetAddressOf());
}
//=============================================================================
#endif // RENDER_D3D11