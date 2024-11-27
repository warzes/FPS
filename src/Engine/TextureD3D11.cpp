#include "stdafx.h"
#if RENDER_D3D11
#include "TextureD3D11.h"
#include "ContextD3D11.h"
//=============================================================================
TextureD3D11::TextureD3D11(uint32_t width, uint32_t height, PixelFormat format, uint32_t mip_count) :
	mWidth(width),
	mHeight(height),
	mFormat(format),
	mMipCount(mip_count)
{
	auto tex_desc = CD3D11_TEXTURE2D_DESC(PixelFormatMap.at(format), width, height);
	tex_desc.MipLevels = mip_count;
	tex_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	tex_desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
	gContext.device->CreateTexture2D(&tex_desc, NULL, mTexture2D.GetAddressOf());

	auto srv_desc = CD3D11_SHADER_RESOURCE_VIEW_DESC(mTexture2D.Get(), D3D11_SRV_DIMENSION_TEXTURE2D);
	gContext.device->CreateShaderResourceView(mTexture2D.Get(), &srv_desc, mShaderResourceView.GetAddressOf());
}
//=============================================================================
TextureD3D11::TextureD3D11(uint32_t width, uint32_t height, PixelFormat format, ComPtr<ID3D11Texture2D> texture) :
	mWidth(width),
	mHeight(height),
	mFormat(format),
	mTexture2D(texture)
{
}
//=============================================================================
void TextureD3D11::Write(uint32_t width, uint32_t height, PixelFormat format, const void* memory,
	uint32_t mip_level, uint32_t offset_x, uint32_t offset_y)
{
	auto channels = GetFormatChannelsCount(format);
	auto channel_size = GetFormatChannelSize(format);
	auto mem_pitch = width * channels * channel_size;
	auto mem_slice_pitch = width * height * channels * channel_size;
	auto dst_box = CD3D11_BOX(offset_x, offset_y, 0, offset_x + width, offset_y + height, 1);
	gContext.context->UpdateSubresource(mTexture2D.Get(), mip_level, &dst_box, memory, mem_pitch,
		mem_slice_pitch);
}
//=============================================================================
void TextureD3D11::GenerateMips()
{
	gContext.context->GenerateMips(mShaderResourceView.Get());
}
//=============================================================================
#endif // RENDER_D3D11