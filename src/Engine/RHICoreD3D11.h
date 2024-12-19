#pragma once

#if RENDER_D3D11

#include "RHICore.h"
#include "RenderCoreD3D.h"
#include "log.h"

class ShaderD3D11;
class Texture2DD3D11;
class RenderTargetD3D11;

inline DXGI_FORMAT ToD3D11(VertexFormat vertexFormat)
{
	switch (vertexFormat)
	{
	case VertexFormat::Float1:           return DXGI_FORMAT_R32_FLOAT;
	case VertexFormat::Float2:           return DXGI_FORMAT_R32G32_FLOAT;
	case VertexFormat::Float3:           return DXGI_FORMAT_R32G32B32_FLOAT;
	case VertexFormat::Float4:           return DXGI_FORMAT_R32G32B32A32_FLOAT;
	case VertexFormat::UChar1:           return DXGI_FORMAT_R8_UINT;
	case VertexFormat::UChar2:           return DXGI_FORMAT_R8G8_UINT;
	case VertexFormat::UChar4:           return DXGI_FORMAT_R8G8B8A8_UINT;
	case VertexFormat::UChar1Normalized: return DXGI_FORMAT_R8_UNORM;
	case VertexFormat::UChar2Normalized: return DXGI_FORMAT_R8G8_UNORM;
	case VertexFormat::UChar4Normalized: return DXGI_FORMAT_R8G8B8A8_UNORM;
	default:
		Fatal("VertexFormat unknown");
		return {};
	}
}

inline DXGI_FORMAT ToD3D11(PixelFormat pixelFormat)
{
	switch (pixelFormat)
	{
	case PixelFormat::R32Float:     return DXGI_FORMAT_R32_FLOAT;
	case PixelFormat::RG32Float:    return DXGI_FORMAT_R32G32_FLOAT;
	case PixelFormat::RGB32Float:   return DXGI_FORMAT_R32G32B32_FLOAT;
	case PixelFormat::RGBA32Float:  return DXGI_FORMAT_R32G32B32A32_FLOAT;
	case PixelFormat::R8UNorm:      return DXGI_FORMAT_R8_UNORM;
	case PixelFormat::RG8UNorm:     return DXGI_FORMAT_R8G8_UNORM;
	case PixelFormat::RGBA8UNorm:   return DXGI_FORMAT_R8G8B8A8_UNORM;

	case PixelFormat::RGB10A2UNorm: return DXGI_FORMAT_R10G10B10A2_UNORM;
	case PixelFormat::BGRA8UNorm:   return DXGI_FORMAT_B8G8R8A8_UNORM;
	default:
		Fatal("PixelFormat unknown");
		return {};
	}
}

inline D3D11_FILTER ToD3D11(Filter sampler)
{
	switch (sampler)
	{
	case Filter::Linear:      return D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	case Filter::Nearest:     return D3D11_FILTER_MIN_MAG_MIP_POINT;
	case Filter::Anisotropic: return D3D11_FILTER_ANISOTROPIC;
	case Filter::LinearPoint: return D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
	default:
		Fatal("Sampler unknown");
		return {};
	}
}

inline D3D11_TEXTURE_ADDRESS_MODE ToD3D11(TextureAddress address)
{
	switch (address)
	{
	case TextureAddress::Wrap:       return D3D11_TEXTURE_ADDRESS_WRAP;
	case TextureAddress::Clamp:      return D3D11_TEXTURE_ADDRESS_CLAMP;
	case TextureAddress::MirrorWrap: return D3D11_TEXTURE_ADDRESS_MIRROR;
	case TextureAddress::Border:     return D3D11_TEXTURE_ADDRESS_BORDER;
	default:
		Fatal("TextureAddress unknown");
		return {};
	}
}

inline D3D11_COMPARISON_FUNC ToD3D11(ComparisonFunc func)
{
	switch (func)
	{
	case ComparisonFunc::Always:       return D3D11_COMPARISON_ALWAYS;
	case ComparisonFunc::Never:        return D3D11_COMPARISON_NEVER;
	case ComparisonFunc::Less:         return D3D11_COMPARISON_LESS;
	case ComparisonFunc::Equal:        return D3D11_COMPARISON_EQUAL;
	case ComparisonFunc::NotEqual:     return D3D11_COMPARISON_NOT_EQUAL;
	case ComparisonFunc::LessEqual:    return D3D11_COMPARISON_LESS_EQUAL;
	case ComparisonFunc::Greater:      return D3D11_COMPARISON_GREATER;
	case ComparisonFunc::GreaterEqual: return D3D11_COMPARISON_GREATER_EQUAL;
	default:
		Fatal("ComparisonFunc unknown");
		return {};
	}
}



#endif // RENDER_D3D11