#pragma once

#if RENDER_D3D11

#include "RHICore.h"

class ShaderD3D11;
class TextureD3D11;
class RenderTargetD3D11;

static const std::unordered_map<VertexFormat, DXGI_FORMAT> VertexFormatMap = {
	{ VertexFormat::Float1, DXGI_FORMAT_R32_FLOAT },
	{ VertexFormat::Float2, DXGI_FORMAT_R32G32_FLOAT },
	{ VertexFormat::Float3, DXGI_FORMAT_R32G32B32_FLOAT },
	{ VertexFormat::Float4, DXGI_FORMAT_R32G32B32A32_FLOAT },
	{ VertexFormat::UChar1Normalized, DXGI_FORMAT_R8_UNORM },
	{ VertexFormat::UChar2Normalized, DXGI_FORMAT_R8G8_UNORM },
	{ VertexFormat::UChar4Normalized, DXGI_FORMAT_R8G8B8A8_UNORM },
	{ VertexFormat::UChar1, DXGI_FORMAT_R8_UINT },
	{ VertexFormat::UChar2, DXGI_FORMAT_R8G8_UINT },
	{ VertexFormat::UChar4, DXGI_FORMAT_R8G8B8A8_UINT },
};

static const std::unordered_map<PixelFormat, DXGI_FORMAT> PixelFormatMap = {
	{ PixelFormat::R32Float, DXGI_FORMAT_R32_FLOAT },
	{ PixelFormat::RG32Float, DXGI_FORMAT_R32G32_FLOAT },
	{ PixelFormat::RGB32Float, DXGI_FORMAT_R32G32B32_FLOAT },
	{ PixelFormat::RGBA32Float, DXGI_FORMAT_R32G32B32A32_FLOAT },
	{ PixelFormat::R8UNorm, DXGI_FORMAT_R8_UNORM },
	{ PixelFormat::RG8UNorm, DXGI_FORMAT_R8G8_UNORM },
	{ PixelFormat::RGBA8UNorm, DXGI_FORMAT_R8G8B8A8_UNORM }
};

#endif // RENDER_D3D11