#pragma once

#if RENDER_D3D11

#include "RenderCoreD3D11.h"

struct DepthStencilStateD3D11
{
	std::optional<DepthMode> depth_mode;
	std::optional<StencilMode> stencil_mode;

	bool operator==(const DepthStencilStateD3D11&) const = default;
};

SE_MAKE_HASHABLE(DepthStencilStateD3D11,
	t.depth_mode,
	t.stencil_mode
);

struct RasterizerStateD3D11
{
	bool scissor_enabled = false;
	CullMode cull_mode = CullMode::None;
	FrontFace front_face = FrontFace::Clockwise;
	std::optional<DepthBias> depth_bias;

	bool operator==(const RasterizerStateD3D11&) const = default;
};

SE_MAKE_HASHABLE(RasterizerStateD3D11,
	t.cull_mode,
	t.scissor_enabled,
	t.front_face,
	t.depth_bias
);

struct SamplerStateD3D11
{
	Sampler sampler = Sampler::Linear;
	TextureAddress texture_address = TextureAddress::Clamp;

	bool operator==(const SamplerStateD3D11&) const = default;
};

SE_MAKE_HASHABLE(SamplerStateD3D11,
	t.sampler,
	t.texture_address
);

#endif // RENDER_D3D11