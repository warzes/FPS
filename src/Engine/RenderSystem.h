#pragma once

#include "WindowPrivateData.h"
#include "RenderCore.h"

struct RenderSystemCreateInfo final
{
	std::unordered_set<RenderFeature> features{};
	bool vsync{ false };
};

class RenderSystem final
{
public:
	~RenderSystem();

	bool Create(const WindowPrivateData& data, const RenderSystemCreateInfo& createInfo);
	void Destroy();

	void Resize(uint32_t width, uint32_t height);

	void SetTopology(Topology topology);
	//void SetViewport(std::optional<Viewport> viewport);
	//void SetScissor(std::optional<Scissor> scissor);
	//void SetTexture(uint32_t binding, TextureHandle* handle);
	//void SetRenderTarget(const RenderTarget** render_target, size_t count);
	//void SetShader(ShaderHandle* handle);
	//void SetInputLayout(const std::vector<InputLayout>& value);
	//void SetVertexBuffer(const VertexBuffer** vertex_buffer, size_t count);
	//void SetIndexBuffer(IndexBufferHandle* handle);
	//void SetUniformBuffer(uint32_t binding, UniformBufferHandle* handle);
	//void SetBlendMode(const std::optional<BlendMode>& blend_mode);
	//void SetDepthMode(const std::optional<DepthMode>& depth_mode);
	//void SetStencilMode(const std::optional<StencilMode>& stencil_mode);
	//void SetCullMode(CullMode cull_mode);
	//void SetSampler(Sampler value);
	//void SetTextureAddress(TextureAddress value);
	//void SetFrontFace(FrontFace value);
	//void SetDepthBias(const std::optional<DepthBias> depth_bias);

	void Present();

private:
	bool createAPI(const WindowPrivateData& data, const RenderSystemCreateInfo& createInfo);
	void destroyAPI();
};