#pragma once

#if RENDER_D3D11

#include "PipelineStateD3D11.h"

class RenderContext final
{
public:
	void Reset();

	uint32_t    GetBackBufferWidth() const;
	uint32_t    GetBackBufferHeight() const;
	PixelFormat GetBackBufferFormat() const;

	ComPtr<IDXGIAdapter4>             adapter{ nullptr };
	ComPtr<ID3D11Device5>             device{ nullptr };
	ComPtr<ID3D11DeviceContext4>      context{ nullptr };
	ComPtr<ID3DUserDefinedAnnotation> annotation{ nullptr };
	ComPtr<IDXGISwapChain4>           swapChain{ nullptr };

	const PixelFormat                 backBufferFormat{ PixelFormat::RGB10A2UNorm };
	const DXGI_FORMAT                 depthBufferFormat{ DXGI_FORMAT_D24_UNORM_S8_UINT };
	Texture2DD3D11*                   backBufferTexture{ nullptr };
	RenderTargetD3D11*                mainRenderTarget{ nullptr };

	std::vector<RenderTargetD3D11*>   renderTargets;
	std::optional<Viewport>           viewport;

	ShaderD3D11*                      shader = nullptr;
	std::vector<InputLayout>          inputLayouts;

	std::unordered_map<DepthStencilStateD3D11, ComPtr<ID3D11DepthStencilState>> depthStencilStates;
	DepthStencilStateD3D11            depthStencilState;

	std::unordered_map<RasterizerStateD3D11, ComPtr<ID3D11RasterizerState>> rasterizerStates;
	RasterizerStateD3D11              rasterizerState;

	std::unordered_map<SamplerStateD3D11, ComPtr<ID3D11SamplerState>> samplerStates;
	SamplerStateD3D11                 samplerState;

	std::unordered_map<std::optional<BlendMode>, ComPtr<ID3D11BlendState>> blendModes;
	std::optional<BlendMode>          blendMode;

	std::unordered_map<uint32_t, Texture2DD3D11*> textures;

	bool shaderDirty = true;
	bool inputLayoutsDirty = true;
	bool depthStencilStateDirty = true;
	bool rasterizerStateDirty = true;
	bool samplerStateDirty = true;
	bool blendModeDirty = true;
	bool viewportDirty = true;

	bool vsync = false;
};

extern RenderContext gContext;

bool CreateMainRenderTargetD3D11(uint32_t width, uint32_t height);
void DestroyMainRenderTargetD3D11();

void EnsureGraphicsState();

#endif // RENDER_D3D11