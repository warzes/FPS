#include "stdafx.h"
#if RENDER_D3D11
#include "PipelineStateD3D11.h"
#include "ContextD3D11.h"
#include "Log.h"
//=============================================================================
ComPtr<ID3D11RasterizerState> CreateRasterizerStateD3D11(const RasterizerState& state)
{
	D3D11_RASTERIZER_DESC desc = {};
	desc.FillMode              = D3D11_FILL_SOLID;
	desc.CullMode              = ToD3D11(state.cullMode);
	desc.FrontCounterClockwise = state.frontFace == FrontFace::CounterClockwise;
	desc.DepthBiasClamp        = D3D11_DEFAULT_DEPTH_BIAS_CLAMP;
	if (state.depthBias.has_value())
	{
		desc.DepthBias            = (INT)state.depthBias->units;
		desc.SlopeScaledDepthBias = state.depthBias->factor;
	}
	else
	{
		desc.DepthBias            = D3D11_DEFAULT_DEPTH_BIAS;
		desc.SlopeScaledDepthBias = D3D11_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;

	}
	desc.DepthClipEnable       = state.depthClipEnable ? TRUE : FALSE;
	desc.ScissorEnable         = state.scissorEnabled ? TRUE : FALSE;
	desc.MultisampleEnable     = FALSE;
	desc.AntialiasedLineEnable = FALSE;

	ComPtr<ID3D11RasterizerState> d3d11state;
	HRESULT hr = gContext.device->CreateRasterizerState(&desc, d3d11state.GetAddressOf());
	if (FAILED(hr))
	{
		Fatal("CreateRasterizerState() failed: " + DXErrorToStr(hr));
		return nullptr;
	}
	return d3d11state;
}
//=============================================================================
ComPtr<ID3D11SamplerState> CreateSamplerStateD3D11(const SamplerState& state)
{
	D3D11_SAMPLER_DESC desc = {};
	desc.Filter             = ToD3D11(state.filter);
	desc.AddressU           = ToD3D11(state.textureAddress);
	desc.AddressV           = ToD3D11(state.textureAddress);
	desc.AddressW           = ToD3D11(state.textureAddress);
	desc.MipLODBias         = state.mipLODBias;
	desc.MaxAnisotropy      = state.maxAnisotropy;
	desc.ComparisonFunc     = ToD3D11(state.comparisonFunc);
	desc.BorderColor[0]     = state.borderColor[0];
	desc.BorderColor[1]     = state.borderColor[1];
	desc.BorderColor[2]     = state.borderColor[2];
	desc.BorderColor[3]     = state.borderColor[3];
	desc.MinLOD             = state.minLOD;
	desc.MaxLOD             = state.maxLOD;

	ComPtr<ID3D11SamplerState> d3d11state;
	HRESULT hr = gContext.device->CreateSamplerState(&desc, d3d11state.GetAddressOf());
	if (FAILED(hr))
	{
		Fatal("CreateSamplerState() failed: " + DXErrorToStr(hr));
		return nullptr;
	}
	return d3d11state;
}
//=============================================================================
#endif // RENDER_D3D11