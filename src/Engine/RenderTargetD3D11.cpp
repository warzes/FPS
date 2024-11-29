#include "stdafx.h"
#if RENDER_D3D11
#include "RenderTargetD3D11.h"
#include "TextureD3D11.h"
#include "ContextD3D11.h"
#include "Log.h"
//=============================================================================
RenderTargetD3D11::RenderTargetD3D11(uint32_t width, uint32_t height, TextureD3D11* texture) 
	: m_texture(texture)
{
	auto format = PixelFormatMap.at(texture->GetFormat());
	auto rtvDesc = CD3D11_RENDER_TARGET_VIEW_DESC(D3D11_RTV_DIMENSION_TEXTURE2D, format);
	if (FAILED(gContext.device->CreateRenderTargetView(texture->GetD3D11Texture2D().Get(), &rtvDesc, m_renderTargetView.GetAddressOf())))
	{
		Fatal("CreateRenderTargetView() failed");
		return;
	}

	auto texDesc = CD3D11_TEXTURE2D_DESC(DXGI_FORMAT_D24_UNORM_S8_UINT, width, height, 1, 1, D3D11_BIND_DEPTH_STENCIL);
	if (FAILED(gContext.device->CreateTexture2D(&texDesc, NULL, m_depthStencilTexture.GetAddressOf())))
	{
		Fatal("CreateTexture2D() failed");
		return;
	}

	auto dsvDesc = CD3D11_DEPTH_STENCIL_VIEW_DESC(D3D11_DSV_DIMENSION_TEXTURE2D, texDesc.Format);
	if (FAILED(gContext.device->CreateDepthStencilView(m_depthStencilTexture.Get(), &dsvDesc, m_depthStencilView.GetAddressOf())))
	{
		Fatal("CreateDepthStencilView() failed");
		return;
	}
}
//=============================================================================
#endif // RENDER_D3D11