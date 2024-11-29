#pragma once

#if RENDER_D3D11

#include "RenderCoreD3D11.h"

class TextureD3D11 final
{
public:
	TextureD3D11() = delete;
	TextureD3D11(uint32_t width, uint32_t height, PixelFormat format, uint32_t mipCount);
	TextureD3D11(uint32_t width, uint32_t height, PixelFormat format, ComPtr<ID3D11Texture2D> d3d11Texture);

	void Write(uint32_t width, uint32_t height, PixelFormat format, const void* memory, uint32_t mipLevel, uint32_t offsetX, uint32_t offsetY);

	void GenerateMips();

	const auto& GetD3D11Texture2D() const { return m_texture2D; }
	const auto& GetD3D11ShaderResourceView() const { return m_shaderResourceView; }
	auto GetWidth() const { return m_width; }
	auto GetHeight() const { return m_height; }
	auto GetFormat() const { return m_format; }

private:
	ComPtr<ID3D11ShaderResourceView> m_shaderResourceView;
	ComPtr<ID3D11Texture2D>          m_texture2D;
	uint32_t                         m_width{ 0 };
	uint32_t                         m_height{ 0 };
	uint32_t                         m_mipCount{ 0 };
	PixelFormat                      m_format{ PixelFormat::RGBA32Float };
};

#endif // RENDER_D3D11