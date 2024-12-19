#include "stdafx.h"
#if RENDER_D3D11
#include "RenderTargetD3D11.h"
#include "TextureD3D11.h"
#include "ContextD3D11.h"
#include "Log.h"
//=============================================================================
RenderTargetD3D11::RenderTargetD3D11(uint32_t width, uint32_t height, Texture2DD3D11* texture) 
	: m_texture(texture)
{
	assert(texture);

	HRESULT hr = E_FAIL;

	const auto format = ToD3D11(texture->GetFormat());
	const auto rtvDesc = CD3D11_RENDER_TARGET_VIEW_DESC(D3D11_RTV_DIMENSION_TEXTURE2D, format);

	hr = gContext.device->CreateRenderTargetView(texture->GetD3D11Texture2D().Get(), &rtvDesc, m_renderTargetView.GetAddressOf());
	if (FAILED(hr))
	{
		Fatal("CreateRenderTargetView() failed:" + DXErrorToStr(hr));
		return;
	}

	const auto depthStencilDesc = CD3D11_TEXTURE2D_DESC(gContext.depthBufferFormat, width, height, 1, 1, D3D11_BIND_DEPTH_STENCIL);
	hr = gContext.device->CreateTexture2D(&depthStencilDesc, nullptr, m_depthStencilTexture.GetAddressOf());
	if (FAILED(hr))
	{
		Fatal("Create DepthStencil Texture2D failed:" + DXErrorToStr(hr));
		return;
	}

	const auto dsvDesc = CD3D11_DEPTH_STENCIL_VIEW_DESC(D3D11_DSV_DIMENSION_TEXTURE2D, depthStencilDesc.Format);
	hr = gContext.device->CreateDepthStencilView(m_depthStencilTexture.Get(), &dsvDesc, m_depthStencilView.GetAddressOf());
	if (FAILED(hr))
	{
		Fatal("CreateDepthStencilView() failed:" + DXErrorToStr(hr));
		return;
	}
}
//=============================================================================
#endif // RENDER_D3D11