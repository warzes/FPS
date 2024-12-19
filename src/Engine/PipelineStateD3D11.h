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

	Sampler        sampler = Sampler::Linear;
	TextureAddress textureAddress = TextureAddress::Clamp;
	ComparisonFunc comparisonFunc = ComparisonFunc::Never;
};

SE_MAKE_HASHABLE(SamplerStateD3D11,
	t.sampler,
	t.textureAddress,
	t.comparisonFunc
);

#endif // RENDER_D3D11