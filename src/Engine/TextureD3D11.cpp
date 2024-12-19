#include "stdafx.h"
#if RENDER_D3D11
#include "TextureD3D11.h"
#include "ContextD3D11.h"
#include "Log.h"
//=============================================================================
Texture2DD3D11::Texture2DD3D11(uint32_t width, uint32_t height, PixelFormat format, uint32_t mipCount)
	: m_width(width)
	, m_height(height)
	, m_format(format)
	, m_mipCount(mipCount)
{
	HRESULT hr = E_FAIL;

	auto texDesc = CD3D11_TEXTURE2D_DESC(PixelFormatMap.at(format), width, height);
	texDesc.MipLevels = mipCount;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	texDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
	hr = gContext.device->CreateTexture2D(&texDesc, nullptr, m_texture2D.GetAddressOf());
	if (FAILED(hr))
	{
		Fatal("CreateTexture2D() failed: " + DXErrorToStr(hr));
		return;
	}

	auto srvDesc = CD3D11_SHADER_RESOURCE_VIEW_DESC(m_texture2D.Get(), D3D11_SRV_DIMENSION_TEXTURE2D);
	hr = gContext.device->CreateShaderResourceView(m_texture2D.Get(), &srvDesc, m_shaderResourceView.GetAddressOf());
	if (FAILED(hr))
	{
		Fatal("CreateShaderResourceView() failed: " + DXErrorToStr(hr));
		return;
	}
}
//=============================================================================
Texture2DD3D11::Texture2DD3D11(uint32_t width, uint32_t height, PixelFormat format, ComPtr<ID3D11Texture2D> texture)
	: m_width(width)
	, m_height(height)
	, m_format(format)
	, m_texture2D(texture)
{
}
//=============================================================================
void Texture2DD3D11::Write(uint32_t width, uint32_t height, PixelFormat format, const void* memory, uint32_t mipLevel, uint32_t offsetX, uint32_t offsetY)
{
	auto channels      = GetFormatChannelsCount(format);
	auto channelSize   = GetFormatChannelSize(format);
	auto memPitch      = width * channels * channelSize;
	auto memSlicePitch = width * height * channels * channelSize;
	auto dstBox        = CD3D11_BOX(offsetX, offsetY, 0, offsetX + width, offsetY + height, 1);
	gContext.context->UpdateSubresource(m_texture2D.Get(), mipLevel, &dstBox, memory, memPitch, memSlicePitch);
}
//=============================================================================
void Texture2DD3D11::GenerateMips()
{
	gContext.context->GenerateMips(m_shaderResourceView.Get());
}
//=============================================================================
#endif // RENDER_D3D11