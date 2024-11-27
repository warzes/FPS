#pragma once

#if RENDER_D3D11

#include "PipelineStateD3D11.h"

class RenderContext final
{
public:
	ComPtr<IDXGISwapChain> swapchain;
	ComPtr<ID3D11Device> device;
	ComPtr<ID3D11DeviceContext> context;
	TextureD3D11* backbuffer_texture = nullptr;
	RenderTargetD3D11* main_render_target = nullptr;
	std::vector<RenderTargetD3D11*> render_targets;
	ShaderD3D11* shader = nullptr;
	std::vector<InputLayout> input_layouts;

	std::unordered_map<DepthStencilStateD3D11, ComPtr<ID3D11DepthStencilState>> depth_stencil_states;
	DepthStencilStateD3D11 depth_stencil_state;

	std::unordered_map<RasterizerStateD3D11, ComPtr<ID3D11RasterizerState>> rasterizer_states;
	RasterizerStateD3D11 rasterizer_state;

	std::unordered_map<SamplerStateD3D11, ComPtr<ID3D11SamplerState>> sampler_states;
	SamplerStateD3D11 sampler_state;

	std::unordered_map<std::optional<BlendMode>, ComPtr<ID3D11BlendState>> blend_modes;
	std::optional<BlendMode> blend_mode;

	std::optional<Viewport> viewport;

	bool shader_dirty = true;
	bool input_layouts_dirty = true;
	bool depth_stencil_state_dirty = true;
	bool rasterizer_state_dirty = true;
	bool sampler_state_dirty = true;
	bool blend_mode_dirty = true;
	bool viewport_dirty = true;

	bool vsync = false;
	std::unordered_map<uint32_t, TextureD3D11*> textures;

	uint32_t GetBackbufferWidth();
	uint32_t GetBackbufferHeight();
	PixelFormat GetBackbufferFormat();
};

extern RenderContext gContext;

void CreateMainRenderTarget(uint32_t width, uint32_t height);
void DestroyMainRenderTarget();

void EnsureGraphicsState(bool draw_indexed);

#endif // RENDER_D3D11