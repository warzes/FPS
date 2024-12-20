#include "stdafx.h"
#if RENDER_D3D11
#include "RHICoreD3D11.h"
#include "ContextD3D11.h"
#include "Log.h"
//=============================================================================
ComPtr<ID3D11DepthStencilState> CreateDepthStencilStateD3D11(const DepthStencilState& state)
{
	auto depthMode = state.depthMode.value_or(DepthMode());
	auto stencilMode = state.stencilMode.value_or(StencilMode());

	D3D11_DEPTH_STENCIL_DESC desc = {};
	desc.DepthEnable = state.depthMode.has_value();
	desc.DepthWriteMask = depthMode.writeMask ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
	desc.DepthFunc = ToD3D11(depthMode.func);

	desc.StencilEnable = state.stencilMode.has_value();
	desc.StencilReadMask = stencilMode.readMask;
	desc.StencilWriteMask = stencilMode.writeMask;

	desc.FrontFace.StencilDepthFailOp = ToD3D11(stencilMode.depthFailOp);
	desc.FrontFace.StencilFailOp = ToD3D11(stencilMode.failOp);
	desc.FrontFace.StencilFunc = ToD3D11(stencilMode.func);
	desc.FrontFace.StencilPassOp = ToD3D11(stencilMode.passOp);

	desc.BackFace = desc.FrontFace;

	ComPtr<ID3D11DepthStencilState> d3d11state;
	HRESULT hr = gContext.device->CreateDepthStencilState(&desc, d3d11state.GetAddressOf());
	if (FAILED(hr))
	{
		Fatal("CreateDepthStencilState() failed: " + DXErrorToStr(hr));
		return nullptr;
	}
	return d3d11state;
}
//=============================================================================
ComPtr<ID3D11RasterizerState> CreateRasterizerStateD3D11(const RasterizerState& state)
{
	D3D11_RASTERIZER_DESC desc = {};
	desc.FillMode = D3D11_FILL_SOLID;
	desc.CullMode = ToD3D11(state.cullMode);
	desc.FrontCounterClockwise = state.frontFace == FrontFace::CounterClockwise;
	desc.DepthBiasClamp = D3D11_DEFAULT_DEPTH_BIAS_CLAMP;
	if (state.depthBias.has_value())
	{
		desc.DepthBias = (INT)state.depthBias->units;
		desc.SlopeScaledDepthBias = state.depthBias->factor;
	}
	else
	{
		desc.DepthBias = D3D11_DEFAULT_DEPTH_BIAS;
		desc.SlopeScaledDepthBias = D3D11_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;

	}
	desc.DepthClipEnable = state.depthClipEnable ? TRUE : FALSE;
	desc.ScissorEnable = state.scissorEnabled ? TRUE : FALSE;
	desc.MultisampleEnable = FALSE;
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
	desc.Filter = ToD3D11(state.filter);
	desc.AddressU = ToD3D11(state.textureAddress);
	desc.AddressV = ToD3D11(state.textureAddress);
	desc.AddressW = ToD3D11(state.textureAddress);
	desc.MipLODBias = state.mipLODBias;
	desc.MaxAnisotropy = state.maxAnisotropy;
	desc.ComparisonFunc = ToD3D11(state.comparisonFunc);
	desc.BorderColor[0] = state.borderColor[0];
	desc.BorderColor[1] = state.borderColor[1];
	desc.BorderColor[2] = state.borderColor[2];
	desc.BorderColor[3] = state.borderColor[3];
	desc.MinLOD = state.minLOD;
	desc.MaxLOD = state.maxLOD;

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