#include "stdafx.h"
#if RENDER_D3D11
#include "ContextD3D11.h"
#include "TextureD3D11.h"
#include "RenderTargetD3D11.h"
#include "ShaderD3D11.h"
#include "Log.h"
//=============================================================================
void RenderContext::Clear()
{
	DestroyMainRenderTargetD3D11();
	swapChain.Reset();
	context.Reset();
	annotation.Reset();
	device.Reset();
	adapter.Reset();
}
//=============================================================================
uint32_t RenderContext::GetBackBufferWidth()
{
	return renderTargets.at(0)->GetTexture()->GetWidth();
}
//=============================================================================
uint32_t RenderContext::GetBackBufferHeight()
{
	return renderTargets.at(0)->GetTexture()->GetHeight();
}
//=============================================================================
PixelFormat RenderContext::GetBackbufferFormat()
{
	return renderTargets.at(0)->GetTexture()->GetFormat();
}
//=============================================================================
bool CreateMainRenderTargetD3D11(uint32_t width, uint32_t height)
{
	HRESULT hr = E_FAIL;

	ComPtr<ID3D11Texture2D> backBuffer;
	hr = gContext.swapChain->GetBuffer(0, IID_PPV_ARGS(backBuffer.GetAddressOf()));
	if (FAILED(hr))
	{
		Fatal("GetBuffer() failed: " + DXErrorToStr(hr));
		return false;
	}

	gContext.backBufferTexture = new TextureD3D11(width, height, PixelFormat::RGBA8UNorm, backBuffer);
	gContext.mainRenderTarget = new RenderTargetD3D11(width, height, gContext.backBufferTexture);

	return true;
}
//=============================================================================
void DestroyMainRenderTargetD3D11()
{
	if (gContext.context)
	{
		// Clear the previous window size specific context.
		gContext.context->OMSetRenderTargets(0, nullptr, nullptr);
		gContext.context->Flush();
	}

	delete gContext.backBufferTexture;
	delete gContext.mainRenderTarget;
	gContext.backBufferTexture = nullptr;
	gContext.mainRenderTarget = nullptr;
}
//=============================================================================
void EnsureShader()
{
	if (!gContext.shader_dirty)
		return;

	gContext.shader_dirty = false;

	gContext.context->VSSetShader(gContext.shader->GetD3D11VertexShader().Get(), NULL, 0);
	gContext.context->PSSetShader(gContext.shader->GetD3D11PixelShader().Get(), NULL, 0);
}
//=============================================================================
void EnsureInputLayout()
{
	if (!gContext.input_layouts_dirty)
		return;

	gContext.input_layouts_dirty = false;

	auto& cache = gContext.shader->GetInputLayoutCache();

	if (!cache.contains(gContext.input_layouts))
	{
		std::vector<D3D11_INPUT_ELEMENT_DESC> input_elements;

		for (size_t i = 0; i < gContext.input_layouts.size(); i++)
		{
			const auto& input_layout = gContext.input_layouts.at(i);

			for (const auto& [location, attribute] : input_layout.attributes)
			{
				static const std::unordered_map<InputLayout::Rate, D3D11_INPUT_CLASSIFICATION> InputRateMap = {
					{ InputLayout::Rate::Vertex, D3D11_INPUT_PER_VERTEX_DATA },
					{ InputLayout::Rate::Instance, D3D11_INPUT_PER_INSTANCE_DATA },
				};

				input_elements.push_back(D3D11_INPUT_ELEMENT_DESC{
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

		gContext.device->CreateInputLayout(input_elements.data(), (UINT)input_elements.size(),
			gContext.shader->GetVertexShaderBlob()->GetBufferPointer(),
			gContext.shader->GetVertexShaderBlob()->GetBufferSize(), cache[gContext.input_layouts].GetAddressOf());
	}

	gContext.context->IASetInputLayout(cache.at(gContext.input_layouts).Get());
}
//=============================================================================
void EnsureDepthStencilState()
{
	if (!gContext.depth_stencil_state_dirty)
		return;

	gContext.depth_stencil_state_dirty = false;

	const auto& depth_stencil_state = gContext.depth_stencil_state;

	auto depth_mode = depth_stencil_state.depthMode.value_or(DepthMode());
	auto stencil_mode = depth_stencil_state.stencilMode.value_or(StencilMode());

	if (!gContext.depth_stencil_states.contains(depth_stencil_state))
	{
		const static std::unordered_map<ComparisonFunc, D3D11_COMPARISON_FUNC> ComparisonFuncMap = {
			{ ComparisonFunc::Always, D3D11_COMPARISON_ALWAYS },
			{ ComparisonFunc::Never, D3D11_COMPARISON_NEVER },
			{ ComparisonFunc::Less, D3D11_COMPARISON_LESS },
			{ ComparisonFunc::Equal, D3D11_COMPARISON_EQUAL },
			{ ComparisonFunc::NotEqual, D3D11_COMPARISON_NOT_EQUAL },
			{ ComparisonFunc::LessEqual, D3D11_COMPARISON_LESS_EQUAL },
			{ ComparisonFunc::Greater, D3D11_COMPARISON_GREATER },
			{ ComparisonFunc::GreaterEqual, D3D11_COMPARISON_GREATER_EQUAL }
		};

		const static std::unordered_map<StencilOp, D3D11_STENCIL_OP> StencilOpMap = {
			{ StencilOp::Keep, D3D11_STENCIL_OP_KEEP },
			{ StencilOp::Zero, D3D11_STENCIL_OP_ZERO },
			{ StencilOp::Replace, D3D11_STENCIL_OP_REPLACE },
			{ StencilOp::IncrementSaturation, D3D11_STENCIL_OP_INCR_SAT },
			{ StencilOp::DecrementSaturation, D3D11_STENCIL_OP_DECR_SAT },
			{ StencilOp::Invert, D3D11_STENCIL_OP_INVERT },
			{ StencilOp::Increment, D3D11_STENCIL_OP_INCR },
			{ StencilOp::Decrement, D3D11_STENCIL_OP_DECR },
		};

		auto desc = CD3D11_DEPTH_STENCIL_DESC(D3D11_DEFAULT);
		desc.DepthEnable = depth_stencil_state.depthMode.has_value();
		desc.DepthFunc = ComparisonFuncMap.at(depth_mode.func);
		desc.DepthWriteMask = depth_mode.writeMask ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;

		desc.StencilEnable = depth_stencil_state.stencilMode.has_value();
		desc.StencilReadMask = stencil_mode.readMask;
		desc.StencilWriteMask = stencil_mode.writeMask;

		desc.FrontFace.StencilDepthFailOp = StencilOpMap.at(stencil_mode.depthFailOp);
		desc.FrontFace.StencilFailOp = StencilOpMap.at(stencil_mode.failOp);
		desc.FrontFace.StencilFunc = ComparisonFuncMap.at(stencil_mode.func);
		desc.FrontFace.StencilPassOp = StencilOpMap.at(stencil_mode.passOp);

		desc.BackFace = desc.FrontFace;

		gContext.device->CreateDepthStencilState(&desc, gContext.depth_stencil_states[depth_stencil_state].GetAddressOf());
	}

	gContext.context->OMSetDepthStencilState(gContext.depth_stencil_states.at(depth_stencil_state).Get(), stencil_mode.reference);
}
//=============================================================================
void EnsureRasterizerState()
{
	if (!gContext.rasterizer_state_dirty)
		return;

	gContext.rasterizer_state_dirty = false;

	const auto& value = gContext.rasterizer_state;

	if (!gContext.rasterizer_states.contains(value))
	{
		const static std::unordered_map<CullMode, D3D11_CULL_MODE> CullMap = {
			{ CullMode::None, D3D11_CULL_NONE },
			{ CullMode::Front, D3D11_CULL_FRONT },
			{ CullMode::Back, D3D11_CULL_BACK }
		};

		auto desc = CD3D11_RASTERIZER_DESC(D3D11_DEFAULT);
		desc.CullMode = CullMap.at(value.cullMode);
		desc.ScissorEnable = value.scissorEnabled;
		desc.FrontCounterClockwise = value.frontFace == FrontFace::CounterClockwise;
		if (value.depthBias.has_value())
		{
			desc.SlopeScaledDepthBias = value.depthBias->factor;
			desc.DepthBias = (INT)value.depthBias->units;
		}
		else
		{
			desc.DepthBias = D3D11_DEFAULT_DEPTH_BIAS;
			desc.SlopeScaledDepthBias = D3D11_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
		}
		gContext.device->CreateRasterizerState(&desc, gContext.rasterizer_states[value].GetAddressOf());
	}

	gContext.context->RSSetState(gContext.rasterizer_states.at(value).Get());
}
//=============================================================================
void EnsureSamplerState()
{
	if (!gContext.sampler_state_dirty)
		return;

	gContext.sampler_state_dirty = false;

	const auto& value = gContext.sampler_state;

	if (!gContext.sampler_states.contains(value))
	{
		// TODO: see D3D11_ENCODE_BASIC_FILTER

		const static std::unordered_map<Sampler, D3D11_FILTER> SamplerMap = {
			{ Sampler::Linear, D3D11_FILTER_MIN_MAG_MIP_LINEAR },
			{ Sampler::Nearest, D3D11_FILTER_MIN_MAG_MIP_POINT },
		};

		const static std::unordered_map<TextureAddress, D3D11_TEXTURE_ADDRESS_MODE> TextureAddressMap = {
			{ TextureAddress::Clamp, D3D11_TEXTURE_ADDRESS_CLAMP },
			{ TextureAddress::Wrap, D3D11_TEXTURE_ADDRESS_WRAP },
			{ TextureAddress::MirrorWrap, D3D11_TEXTURE_ADDRESS_MIRROR }
		};

		auto desc = CD3D11_SAMPLER_DESC(D3D11_DEFAULT);
		desc.Filter = SamplerMap.at(value.sampler);
		desc.AddressU = TextureAddressMap.at(value.textureAddress);
		desc.AddressV = TextureAddressMap.at(value.textureAddress);
		desc.AddressW = TextureAddressMap.at(value.textureAddress);
		gContext.device->CreateSamplerState(&desc, gContext.sampler_states[value].GetAddressOf());
	}

	for (auto [binding, _] : gContext.textures)
	{
		gContext.context->PSSetSamplers(binding, 1, gContext.sampler_states.at(value).GetAddressOf());
	}
}
//=============================================================================
void EnsureBlendMode()
{
	if (!gContext.blend_mode_dirty)
		return;

	gContext.blend_mode_dirty = false;

	const auto& blend_mode = gContext.blend_mode;

	if (!gContext.blend_modes.contains(blend_mode))
	{
		const static std::unordered_map<Blend, D3D11_BLEND> ColorBlendMap = {
			{ Blend::One, D3D11_BLEND_ONE },
			{ Blend::Zero, D3D11_BLEND_ZERO },
			{ Blend::SrcColor, D3D11_BLEND_SRC_COLOR },
			{ Blend::InvSrcColor, D3D11_BLEND_INV_SRC_COLOR },
			{ Blend::SrcAlpha, D3D11_BLEND_SRC_ALPHA },
			{ Blend::InvSrcAlpha, D3D11_BLEND_INV_SRC_ALPHA },
			{ Blend::DstColor, D3D11_BLEND_DEST_COLOR },
			{ Blend::InvDstColor, D3D11_BLEND_INV_DEST_COLOR },
			{ Blend::DstAlpha, D3D11_BLEND_DEST_ALPHA },
			{ Blend::InvDstAlpha, D3D11_BLEND_INV_DEST_ALPHA }
		};

		const static std::unordered_map<Blend, D3D11_BLEND> AlphaBlendMap = {
			{ Blend::One, D3D11_BLEND_ONE },
			{ Blend::Zero, D3D11_BLEND_ZERO },
			{ Blend::SrcColor, D3D11_BLEND_SRC_ALPHA },
			{ Blend::InvSrcColor, D3D11_BLEND_INV_SRC_ALPHA },
			{ Blend::SrcAlpha, D3D11_BLEND_SRC_ALPHA },
			{ Blend::InvSrcAlpha, D3D11_BLEND_INV_SRC_ALPHA },
			{ Blend::DstColor, D3D11_BLEND_DEST_ALPHA },
			{ Blend::InvDstColor, D3D11_BLEND_INV_DEST_ALPHA },
			{ Blend::DstAlpha, D3D11_BLEND_DEST_ALPHA },
			{ Blend::InvDstAlpha, D3D11_BLEND_INV_DEST_ALPHA }
		};

		const static std::unordered_map<BlendFunction, D3D11_BLEND_OP> BlendOpMap = {
			{ BlendFunction::Add, D3D11_BLEND_OP_ADD },
			{ BlendFunction::Subtract, D3D11_BLEND_OP_SUBTRACT },
			{ BlendFunction::ReverseSubtract, D3D11_BLEND_OP_REV_SUBTRACT },
			{ BlendFunction::Min, D3D11_BLEND_OP_MIN },
			{ BlendFunction::Max, D3D11_BLEND_OP_MAX },
		};

		auto desc = CD3D11_BLEND_DESC(D3D11_DEFAULT);

		for (int i = 0; i < D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT; i++)
		{
			auto& blend = desc.RenderTarget[i];

			blend.BlendEnable = blend_mode.has_value();

			if (!blend.BlendEnable)
				continue;

			const auto& blend_mode_nn = blend_mode.value();

			if (blend_mode_nn.colorMask.red)
				blend.RenderTargetWriteMask |= D3D11_COLOR_WRITE_ENABLE_RED;

			if (blend_mode_nn.colorMask.green)
				blend.RenderTargetWriteMask |= D3D11_COLOR_WRITE_ENABLE_GREEN;

			if (blend_mode_nn.colorMask.blue)
				blend.RenderTargetWriteMask |= D3D11_COLOR_WRITE_ENABLE_BLUE;

			if (blend_mode_nn.colorMask.alpha)
				blend.RenderTargetWriteMask |= D3D11_COLOR_WRITE_ENABLE_ALPHA;

			blend.SrcBlend = ColorBlendMap.at(blend_mode_nn.colorSrc);
			blend.DestBlend = ColorBlendMap.at(blend_mode_nn.colorDst);
			blend.BlendOp = BlendOpMap.at(blend_mode_nn.colorFunc);

			blend.SrcBlendAlpha = AlphaBlendMap.at(blend_mode_nn.alphaSrc);
			blend.DestBlendAlpha = AlphaBlendMap.at(blend_mode_nn.alphaDst);
			blend.BlendOpAlpha = BlendOpMap.at(blend_mode_nn.alphaFunc);
		}

		gContext.device->CreateBlendState(&desc, gContext.blend_modes[blend_mode].GetAddressOf());
	}

	gContext.context->OMSetBlendState(gContext.blend_modes.at(blend_mode).Get(), nullptr, 0xFFFFFFFF);
}
//=============================================================================
void EnsureViewport()
{
	if (!gContext.viewport_dirty)
		return;

	gContext.viewport_dirty = false;

	auto width = static_cast<float>(gContext.GetBackBufferWidth());
	auto height = static_cast<float>(gContext.GetBackBufferHeight());

	auto viewport = gContext.viewport.value_or(Viewport{ { 0.0f, 0.0f }, { width, height } });

	D3D11_VIEWPORT vp;
	vp.Width = viewport.size.x;
	vp.Height = viewport.size.y;
	vp.MinDepth = viewport.minDepth;
	vp.MaxDepth = viewport.maxDepth;
	vp.TopLeftX = viewport.position.x;
	vp.TopLeftY = viewport.position.y;
	gContext.context->RSSetViewports(1, &vp);
}
//=============================================================================
void EnsureGraphicsState(bool draw_indexed)
{
	EnsureShader();
	EnsureInputLayout();
	EnsureDepthStencilState();
	EnsureRasterizerState();
	EnsureSamplerState();
	EnsureBlendMode();
	EnsureViewport();
}
//=============================================================================
#endif // RENDER_D3D11