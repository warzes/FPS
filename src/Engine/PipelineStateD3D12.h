#pragma once

#if RENDER_D3D12

#include "RenderCoreD3D12.h"

struct RasterizerStateD3D12 final
{
	CullMode  cullMode = CullMode::None;
	FrontFace frontFace = FrontFace::Clockwise;

	bool operator==(const RasterizerStateD3D12&) const = default;
};

SE_MAKE_HASHABLE(RasterizerStateD3D12,
	t.cullMode,
	t.frontFace
);

struct PipelineStateD3D12 final
{
	ShaderD3D12*               shader = nullptr;
	RasterizerStateD3D12       rasterizerState;
	std::optional<DepthMode>   depthMode;
	std::optional<BlendMode>   blendMode;
	TopologyKind               topologyKind = TopologyKind::Triangles;
	std::vector<DXGI_FORMAT>   colorAttachmentFormats;
	std::optional<DXGI_FORMAT> depthStencilFormat;
	std::vector<InputLayout>   inputLayouts;

	bool operator==(const PipelineStateD3D12&) const = default;
};

SE_MAKE_HASHABLE(PipelineStateD3D12,
	t.shader,
	t.rasterizerState,
	t.depthMode,
	t.blendMode,
	t.topologyKind,
	t.colorAttachmentFormats,
	t.depthStencilFormat,
	t.inputLayouts
);

ComPtr<ID3D12PipelineState> CreateGraphicsPipelineState(const PipelineStateD3D12& pipeline_state); // TODO:

#endif // RENDER_D3D12