#include "stdafx.h"
#if RENDER_D3D11
#include "ContextD3D11.h"
#include "TextureD3D11.h"
#include "RenderTargetD3D11.h"
#include "ShaderD3D11.h"
#include "Log.h"
//=============================================================================
void RenderContext::Reset()
{
	renderTargets.clear();
	viewport.reset();
	shader = nullptr;
	inputLayouts.clear();
	cacheDepthStencilStates.clear();
	depthStencilState = {};
	cacheRasterizerStates.clear();
	rasterizerState = {};
	cacheSamplerStates.clear();
	samplerState = {};
	cacheBlendModes.clear();
	blendMode.reset();
	textures.clear();

	shaderDirty = true;
	inputLayoutsDirty = true;
	depthStencilStateDirty = true;
	rasterizerStateDirty = true;
	samplerStateDirty = true;
	blendModeDirty = true;
	viewportDirty = true;

	DestroyMainRenderTargetD3D11();
	swapChain.Reset();
	context.Reset();
	annotation.Reset();
	device.Reset();
	adapter.Reset();
}
//=============================================================================
uint32_t RenderContext::GetBackBufferWidth() const
{
	return renderTargets.at(0)->GetTexture()->GetWidth();
}
//=============================================================================
uint32_t RenderContext::GetBackBufferHeight() const
{
	return renderTargets.at(0)->GetTexture()->GetHeight();
}
//=============================================================================
PixelFormat RenderContext::GetBackBufferFormat() const
{
	return renderTargets.at(0)->GetTexture()->GetFormat();
}
//=============================================================================
bool CreateMainRenderTargetD3D11(uint32_t width, uint32_t height)
{
	ComPtr<ID3D11Texture2D> backBuffer;
	HRESULT hr = gContext.swapChain->GetBuffer(0, IID_PPV_ARGS(backBuffer.GetAddressOf()));
	if (FAILED(hr))
	{
		Fatal("IDXGISwapChain::GetBuffer() failed: " + DXErrorToStr(hr));
		return false;
	}

	gContext.backBufferTexture = new Texture2DD3D11(width, height, gContext.backBufferFormat, backBuffer);
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
void ensureShader()
{
	if (!gContext.shaderDirty) return;
	gContext.shaderDirty = false;

	gContext.context->VSSetShader(gContext.shader->GetD3D11VertexShader().Get(), nullptr, 0);
	gContext.context->PSSetShader(gContext.shader->GetD3D11PixelShader().Get(), nullptr, 0);
}
//=============================================================================
void ensureInputLayout()
{
	if (!gContext.inputLayoutsDirty) return;
	gContext.inputLayoutsDirty = false;

	assert(gContext.shader);

	auto& cache = gContext.shader->GetInputLayoutCache();

	if (!cache.contains(gContext.inputLayouts))
	{
		std::vector<D3D11_INPUT_ELEMENT_DESC> inputElements;

		for (size_t i = 0; i < gContext.inputLayouts.size(); i++)
		{
			const auto& input_layout = gContext.inputLayouts.at(i);

			for (const auto& [location, attribute] : input_layout.attributes)
			{
				static const std::unordered_map<InputLayout::Rate, D3D11_INPUT_CLASSIFICATION> InputRateMap = {
					{ InputLayout::Rate::Vertex, D3D11_INPUT_PER_VERTEX_DATA },
					{ InputLayout::Rate::Instance, D3D11_INPUT_PER_INSTANCE_DATA },
				};

				inputElements.push_back(D3D11_INPUT_ELEMENT_DESC{
					.SemanticName = "TEXCOORD",
					.SemanticIndex = (UINT)location,
					.Format = ToD3D11(attribute.format),
					.InputSlot = (UINT)i,
					.AlignedByteOffset = (UINT)attribute.offset,
					.InputSlotClass = InputRateMap.at(input_layout.rate),
					.InstanceDataStepRate = (UINT)(input_layout.rate == InputLayout::Rate::Vertex ? 0 : 1)
					});
			}
		}

		HRESULT hr = gContext.device->CreateInputLayout(
			inputElements.data(), static_cast<UINT>(inputElements.size()),
			gContext.shader->GetVertexShaderBlob()->GetBufferPointer(),
			gContext.shader->GetVertexShaderBlob()->GetBufferSize(), cache[gContext.inputLayouts].GetAddressOf());
		if (FAILED(hr))
		{
			Fatal("CreateInputLayout() failed: " + DXErrorToStr(hr));
			return;
		}
	}

	gContext.context->IASetInputLayout(cache.at(gContext.inputLayouts).Get());
}
//=============================================================================
void ensureDepthStencilState()
{
	if (!gContext.depthStencilStateDirty) return;
	gContext.depthStencilStateDirty = false;

	const auto& depth_stencil_state = gContext.depthStencilState;

	auto depth_mode = depth_stencil_state.depthMode.value_or(DepthMode());
	auto stencil_mode = depth_stencil_state.stencilMode.value_or(StencilMode());

	if (!gContext.cacheDepthStencilStates.contains(depth_stencil_state))
	{


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
		desc.DepthFunc = ToD3D11(depth_mode.func);
		desc.DepthWriteMask = depth_mode.writeMask ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;

		desc.StencilEnable = depth_stencil_state.stencilMode.has_value();
		desc.StencilReadMask = stencil_mode.readMask;
		desc.StencilWriteMask = stencil_mode.writeMask;

		desc.FrontFace.StencilDepthFailOp = StencilOpMap.at(stencil_mode.depthFailOp);
		desc.FrontFace.StencilFailOp = StencilOpMap.at(stencil_mode.failOp);
		desc.FrontFace.StencilFunc = ToD3D11(stencil_mode.func);
		desc.FrontFace.StencilPassOp = StencilOpMap.at(stencil_mode.passOp);

		desc.BackFace = desc.FrontFace;

		HRESULT hr = gContext.device->CreateDepthStencilState(&desc, gContext.cacheDepthStencilStates[depth_stencil_state].GetAddressOf());
		if (FAILED(hr))
		{
			Fatal("CreateDepthStencilState() failed: " + DXErrorToStr(hr));
		}
	}

	gContext.context->OMSetDepthStencilState(gContext.cacheDepthStencilStates.at(depth_stencil_state).Get(), stencil_mode.reference);
}
//=============================================================================
void ensureRasterizerState()
{
	if (!gContext.rasterizerStateDirty) return;
	gContext.rasterizerStateDirty = false;

	const auto& value = gContext.rasterizerState;

	if (!gContext.cacheRasterizerStates.contains(value))
	{
		const static std::unordered_map<CullMode, D3D11_CULL_MODE> CullMap = {
			{ CullMode::None, D3D11_CULL_NONE },
			{ CullMode::Front, D3D11_CULL_FRONT },
			{ CullMode::Back, D3D11_CULL_BACK }
		};

		auto desc = CD3D11_RASTERIZER_DESC(D3D11_DEFAULT);
		desc.CullMode              = CullMap.at(value.cullMode);
		desc.ScissorEnable         = value.scissorEnabled;
		desc.FrontCounterClockwise = value.frontFace == FrontFace::CounterClockwise;
		if (value.depthBias.has_value())
		{
			desc.SlopeScaledDepthBias = value.depthBias->factor;
			desc.DepthBias            = (INT)value.depthBias->units;
		}
		else
		{
			desc.SlopeScaledDepthBias = D3D11_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
			desc.DepthBias            = D3D11_DEFAULT_DEPTH_BIAS;
		}
		HRESULT hr = gContext.device->CreateRasterizerState(&desc, gContext.cacheRasterizerStates[value].GetAddressOf());
		if (FAILED(hr))
		{
			Fatal("CreateRasterizerState() failed: " + DXErrorToStr(hr));
			return;
		}
	}

	gContext.context->RSSetState(gContext.cacheRasterizerStates.at(value).Get());
}
//=============================================================================
void ensureSamplerState()
{
	if (!gContext.samplerStateDirty) return;
	gContext.samplerStateDirty = false;

	const auto& value = gContext.samplerState;

	if (!gContext.cacheSamplerStates.contains(value))
	{
		auto state = CreateSamplerStateD3D11(value);
		if (!state) return;
		gContext.cacheSamplerStates[value] = state;
	}

	for (auto [binding, _] : gContext.textures)
	{
		gContext.context->PSSetSamplers(binding, 1, gContext.cacheSamplerStates.at(value).GetAddressOf());
	}
}
//=============================================================================
void ensureBlendMode()
{
	if (!gContext.blendModeDirty) return;
	gContext.blendModeDirty = false;

	const auto& blendMode = gContext.blendMode;

	if (!gContext.cacheBlendModes.contains(blendMode))
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

			blend.BlendEnable = blendMode.has_value();

			if (!blend.BlendEnable)
				continue;

			const auto& blend_mode_nn = blendMode.value();

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

		HRESULT hr = gContext.device->CreateBlendState(&desc, gContext.cacheBlendModes[blendMode].GetAddressOf());
		if (FAILED(hr))
		{
			Fatal("CreateBlendState() failed: " + DXErrorToStr(hr));
			return;
		}
	}

	gContext.context->OMSetBlendState(gContext.cacheBlendModes.at(blendMode).Get(), nullptr, 0xFFFFFFFF);
}
//=============================================================================
void ensureViewport()
{
	if (!gContext.viewportDirty) return;
	gContext.viewportDirty = false;

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
void EnsureGraphicsState()
{
	ensureShader();
	ensureInputLayout();
	ensureDepthStencilState();
	ensureRasterizerState();
	ensureSamplerState();
	ensureBlendMode();
	ensureViewport();
}
//=============================================================================
#endif // RENDER_D3D11