#include "stdafx.h"
#if RENDER_D3D11
#include "PipelineStateD3D11.h"
#include "ContextD3D11.h"
#include "Log.h"
//=============================================================================
ComPtr<ID3D11SamplerState> CreateSamplerStateD3D11(const SamplerStateD3D11& sampler)
{
	D3D11_SAMPLER_DESC desc = {};
	desc.Filter             = ToD3D11(sampler.filter);
	desc.AddressU           = ToD3D11(sampler.textureAddress);
	desc.AddressV           = ToD3D11(sampler.textureAddress);
	desc.AddressW           = ToD3D11(sampler.textureAddress);
	desc.MipLODBias         = sampler.mipLODBias;
	desc.MaxAnisotropy      = sampler.maxAnisotropy;
	desc.ComparisonFunc     = ToD3D11(sampler.comparisonFunc);
	desc.BorderColor[0]     = sampler.borderColor[0];
	desc.BorderColor[1]     = sampler.borderColor[1];
	desc.BorderColor[2]     = sampler.borderColor[2];
	desc.BorderColor[3]     = sampler.borderColor[3];
	desc.MinLOD             = sampler.minLOD;
	desc.MaxLOD             = sampler.maxLOD;

	ComPtr<ID3D11SamplerState> state;
	HRESULT hr = gContext.device->CreateSamplerState(&desc, state.GetAddressOf());
	if (FAILED(hr))
	{
		Fatal("CreateSamplerState() failed: " + DXErrorToStr(hr));
		return nullptr;
	}
	return state;
}
//=============================================================================
#endif // RENDER_D3D11