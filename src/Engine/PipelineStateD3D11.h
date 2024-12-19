#pragma once

#if RENDER_D3D11

#include "RHICoreD3D11.h"

struct DepthStencilStateD3D11 final
{
	std::optional<DepthMode>   depthMode;
	std::optional<StencilMode> stencilMode;

	bool operator==(const DepthStencilStateD3D11&) const = default;
};

SE_MAKE_HASHABLE(DepthStencilStateD3D11,
	t.depthMode,
	t.stencilMode
);

struct RasterizerStateD3D11 final
{
	bool                     scissorEnabled = false;
	CullMode                 cullMode = CullMode::None;
	FrontFace                frontFace = FrontFace::Clockwise;
	std::optional<DepthBias> depthBias;

	bool operator==(const RasterizerStateD3D11&) const = default;
};

SE_MAKE_HASHABLE(RasterizerStateD3D11,
	t.cullMode,
	t.scissorEnabled,
	t.frontFace,
	t.depthBias
);

struct SamplerStateD3D11 final
{
	bool operator==(const SamplerStateD3D11&) const = default;

	Filter         filter{ Filter::Linear };
	TextureAddress textureAddress{ TextureAddress::Clamp };
	float          mipLODBias{ 0.0f };
	uint32_t       maxAnisotropy{ 1 };
	ComparisonFunc comparisonFunc{ ComparisonFunc::Never };
	float          borderColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	float          minLOD{ -FLT_MAX };
	float          maxLOD{ FLT_MAX };
};

ComPtr<ID3D11SamplerState> CreateSamplerStateD3D11(const SamplerStateD3D11& sampler);

SE_MAKE_HASHABLE(SamplerStateD3D11,
	t.filter,
	t.textureAddress,
	t.mipLODBias,
	t.maxAnisotropy,
	t.comparisonFunc,
	t.borderColor[0],
	t.borderColor[1],
	t.borderColor[2],
	t.borderColor[3],
	t.minLOD,
	t.maxLOD
);

#endif // RENDER_D3D11