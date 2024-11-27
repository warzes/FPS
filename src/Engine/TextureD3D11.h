#pragma once

#if RENDER_D3D11

#include "RenderCoreD3D11.h"

class TextureD3D11
{
public:
	TextureD3D11(uint32_t width, uint32_t height, PixelFormat format, uint32_t mip_count);
	TextureD3D11(uint32_t width, uint32_t height, PixelFormat format, ComPtr<ID3D11Texture2D> texture);

	void Write(uint32_t width, uint32_t height, PixelFormat format, const void* memory, uint32_t mip_level, uint32_t offset_x, uint32_t offset_y);

	void GenerateMips();

	const auto& GetD3D11Texture2D() const { return mTexture2D; }
	const auto& GetD3D11ShaderResourceView() const { return mShaderResourceView; }
	auto GetWidth() const { return mWidth; }
	auto GetHeight() const { return mHeight; }
	auto GetFormat() const { return mFormat; }

private:
	ComPtr<ID3D11ShaderResourceView> mShaderResourceView;
	ComPtr<ID3D11Texture2D> mTexture2D;
	uint32_t mWidth = 0;
	uint32_t mHeight = 0;
	uint32_t mMipCount = 0;
	PixelFormat mFormat;
};

#endif // RENDER_D3D11