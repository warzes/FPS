#include "stdafx.h"
#if RENDER_D3D12
#include "PipelineStateD3D12.h"
#include "ShaderD3D12.h"
#include "ContextD3D12.h"
//=============================================================================
ComPtr<ID3D12PipelineState> CreateGraphicsPipelineState(const PipelineStateD3D12& pipeline_state)
{
	const static std::unordered_map<CullingMode, D3D12_CULL_MODE> CullMap = {
		{ CullingMode::None, D3D12_CULL_MODE_NONE },
		{ CullingMode::Front, D3D12_CULL_MODE_FRONT },
		{ CullingMode::Back, D3D12_CULL_MODE_BACK }
	};

	const static std::unordered_map<ComparisonFunction, D3D12_COMPARISON_FUNC> ComparisonFuncMap = {
		{ ComparisonFunction::Always, D3D12_COMPARISON_FUNC_ALWAYS },
		{ ComparisonFunction::Never, D3D12_COMPARISON_FUNC_NEVER },
		{ ComparisonFunction::Less, D3D12_COMPARISON_FUNC_LESS },
		{ ComparisonFunction::Equal, D3D12_COMPARISON_FUNC_EQUAL },
		{ ComparisonFunction::NotEqual, D3D12_COMPARISON_FUNC_NOT_EQUAL },
		{ ComparisonFunction::LessEqual, D3D12_COMPARISON_FUNC_LESS_EQUAL },
		{ ComparisonFunction::Greater, D3D12_COMPARISON_FUNC_GREATER },
		{ ComparisonFunction::GreaterEqual, D3D12_COMPARISON_FUNC_GREATER_EQUAL }
	};

	const static std::unordered_map<BlendFactor, D3D12_BLEND> BlendMap = {
		{ BlendFactor::One, D3D12_BLEND_ONE },
		{ BlendFactor::Zero, D3D12_BLEND_ZERO },
		{ BlendFactor::SrcColor, D3D12_BLEND_SRC_COLOR },
		{ BlendFactor::InvSrcColor, D3D12_BLEND_INV_SRC_COLOR },
		{ BlendFactor::SrcAlpha, D3D12_BLEND_SRC_ALPHA },
		{ BlendFactor::InvSrcAlpha, D3D12_BLEND_INV_SRC_ALPHA },
		{ BlendFactor::DstColor, D3D12_BLEND_DEST_COLOR },
		{ BlendFactor::InvDstColor, D3D12_BLEND_INV_DEST_COLOR },
		{ BlendFactor::DstAlpha, D3D12_BLEND_DEST_ALPHA },
		{ BlendFactor::InvDstAlpha, D3D12_BLEND_INV_DEST_ALPHA }
	};

	const static std::unordered_map<BlendFunction, D3D12_BLEND_OP> BlendOpMap = {
		{ BlendFunction::Add, D3D12_BLEND_OP_ADD },
		{ BlendFunction::Subtract, D3D12_BLEND_OP_SUBTRACT },
		{ BlendFunction::ReverseSubtract, D3D12_BLEND_OP_REV_SUBTRACT },
		{ BlendFunction::Min, D3D12_BLEND_OP_MIN },
		{ BlendFunction::Max, D3D12_BLEND_OP_MAX },
	};

	auto depth_mode = pipeline_state.depthMode.value_or(DepthMode());
	const auto& blend_mode = pipeline_state.blendMode;

	auto depth_stencil_state = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	depth_stencil_state.DepthEnable = pipeline_state.depthMode.has_value();
	depth_stencil_state.DepthWriteMask = depth_mode.writeMask ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
	depth_stencil_state.DepthFunc = ComparisonFuncMap.at(depth_mode.func);
	depth_stencil_state.StencilEnable = false;

	auto blend_state = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	blend_state.AlphaToCoverageEnable = false;

	for (int i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; i++)
	{
		auto& blend = blend_state.RenderTarget[i];

		blend.BlendEnable = blend_mode.has_value();

		if (!blend.BlendEnable)
			continue;

		const auto& blend_mode_nn = blend_mode.value();

		if (blend_mode_nn.colorMask.red)
			blend.RenderTargetWriteMask |= D3D12_COLOR_WRITE_ENABLE_RED;

		if (blend_mode_nn.colorMask.green)
			blend.RenderTargetWriteMask |= D3D12_COLOR_WRITE_ENABLE_GREEN;

		if (blend_mode_nn.colorMask.blue)
			blend.RenderTargetWriteMask |= D3D12_COLOR_WRITE_ENABLE_BLUE;

		if (blend_mode_nn.colorMask.alpha)
			blend.RenderTargetWriteMask |= D3D12_COLOR_WRITE_ENABLE_ALPHA;

		blend.SrcBlend = BlendMap.at(blend_mode_nn.colorSrc);
		blend.DestBlend = BlendMap.at(blend_mode_nn.colorDst);
		blend.BlendOp = BlendOpMap.at(blend_mode_nn.colorFunc);

		blend.SrcBlendAlpha = BlendMap.at(blend_mode_nn.alphaSrc);
		blend.DestBlendAlpha = BlendMap.at(blend_mode_nn.alphaDst);
		blend.BlendOpAlpha = BlendOpMap.at(blend_mode_nn.alphaFunc);
	}

	auto rasterizer_state = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	rasterizer_state.CullMode = CullMap.at(pipeline_state.rasterizerState.cullMode);
	rasterizer_state.FrontCounterClockwise = pipeline_state.rasterizerState.frontFace == FrontFace::CounterClockwise;

	const static std::unordered_map<TopologyKind, D3D12_PRIMITIVE_TOPOLOGY_TYPE> TopologyTypeMap = {
		{ TopologyKind::Points, D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT },
		{ TopologyKind::Lines, D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE },
		{ TopologyKind::Triangles, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE }
	};

	auto topology_type = TopologyTypeMap.at(pipeline_state.topologyKind);

	std::vector<D3D12_INPUT_ELEMENT_DESC> input_elements;

	for (size_t i = 0; i < pipeline_state.inputLayouts.size(); i++)
	{
		const auto& input_layout = pipeline_state.inputLayouts.at(i);

		for (const auto& [location, attribute] : input_layout.attributes)
		{
			static const std::unordered_map<InputLayout::Rate, D3D12_INPUT_CLASSIFICATION> InputRateMap = {
				{ InputLayout::Rate::Vertex, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA },
				{ InputLayout::Rate::Instance, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA },
			};

			input_elements.push_back(D3D12_INPUT_ELEMENT_DESC{
				.SemanticName = "TEXCOORD",
				.SemanticIndex = (UINT)location,
				.Format = VertexFormatMap.at(attribute.format),
				.InputSlot = (UINT)i,
				.AlignedByteOffset = (UINT)attribute.offset,
				.InputSlotClass = InputRateMap.at(input_layout.rate),
				.InstanceDataStepRate = (UINT)(input_layout.rate == InputLayout::Rate::Vertex ? 0 : 1)
				});
		}
	}

	D3D12_GRAPHICS_PIPELINE_STATE_DESC pso_desc = {};
	pso_desc.VS = CD3DX12_SHADER_BYTECODE(pipeline_state.shader->GetVertexShaderBlob().Get());
	pso_desc.PS = CD3DX12_SHADER_BYTECODE(pipeline_state.shader->GetPixelShaderBlob().Get());
	pso_desc.InputLayout = { input_elements.data(), (UINT)input_elements.size() };
	pso_desc.NodeMask = 1;
	pso_desc.PrimitiveTopologyType = topology_type;
	pso_desc.pRootSignature = pipeline_state.shader->GetRootSignature().Get();
	pso_desc.SampleMask = UINT_MAX;
	pso_desc.NumRenderTargets = (UINT)pipeline_state.colorAttachmentFormats.size();
	for (size_t i = 0; i < pipeline_state.colorAttachmentFormats.size(); i++)
	{
		pso_desc.RTVFormats[i] = pipeline_state.colorAttachmentFormats.at(i);
	}
	pso_desc.DSVFormat = pipeline_state.depthStencilFormat.value();
	pso_desc.SampleDesc.Count = 1;
	pso_desc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	pso_desc.RasterizerState = rasterizer_state;
	pso_desc.DepthStencilState = depth_stencil_state;
	pso_desc.BlendState = blend_state;

	ComPtr<ID3D12PipelineState> result;
	gContext.device->CreateGraphicsPipelineState(&pso_desc, IID_PPV_ARGS(&result));

	return result;
}
//=============================================================================
#endif // RENDER_D3D12