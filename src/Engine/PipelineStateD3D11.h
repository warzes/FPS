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

ComPtr<ID3D11RasterizerState> CreateRasterizerStateD3D11(const RasterizerState& state);
ComPtr<ID3D11SamplerState>    CreateSamplerStateD3D11(const SamplerState& state);


#endif // RENDER_D3D11