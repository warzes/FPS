#pragma once

#include "WindowPrivateData.h"
#include "RenderResources.h"

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
	void SetSize(uint32_t width, uint32_t height);

	TextureHandle* CreateTexture(uint32_t width, uint32_t height, PixelFormat format, uint32_t mip_count);
	void WriteTexturePixels(TextureHandle* handle, uint32_t width, uint32_t height, PixelFormat format, const void* memory, uint32_t mip_level, uint32_t offset_x, uint32_t offset_y);
	void GenerateMips(TextureHandle* handle);
	void DestroyTexture(TextureHandle* handle);

	RenderTargetHandle* CreateRenderTarget(uint32_t width, uint32_t height, TextureHandle* texture);
	void DestroyRenderTarget(RenderTargetHandle* handle);

	ShaderHandle* CreateShader(const std::string& vertex_code, const std::string& fragment_code, const std::vector<std::string>& defines);
	void DestroyShader(ShaderHandle* handle);

	VertexBufferHandle* CreateVertexBuffer(size_t size, size_t stride);
	void DestroyVertexBuffer(VertexBufferHandle* handle);
	void WriteVertexBufferMemory(VertexBufferHandle* handle, const void* memory, size_t size, size_t stride);

	IndexBufferHandle* CreateIndexBuffer(size_t size, size_t stride);
	void DestroyIndexBuffer(IndexBufferHandle* handle);
	void WriteIndexBufferMemory(IndexBufferHandle* handle, const void* memory, size_t size, size_t stride);

	UniformBufferHandle* CreateUniformBuffer(size_t size);
	void DestroyUniformBuffer(UniformBufferHandle* handle);
	void WriteUniformBufferMemory(UniformBufferHandle* handle, const void* memory, size_t size);

	void SetTopology(Topology topology);
	void SetViewport(std::optional<Viewport> viewport);
	void SetScissor(std::optional<Scissor> scissor);
	void SetTexture(uint32_t binding, TextureHandle* handle);
	void SetTexture(uint32_t binding, Texture& texture);
	void SetRenderTarget(const RenderTarget** render_target, size_t count);
	void SetRenderTarget(const std::vector<const RenderTarget*>& value);
	void SetRenderTarget(const RenderTarget& value);
	void SetRenderTarget(std::nullopt_t value);
	void SetShader(ShaderHandle* handle);
	void SetShader(Shader& shader);
	void SetInputLayout(const InputLayout& value);
	void SetInputLayout(const std::vector<InputLayout>& value);
	
	void SetVertexBuffer(const std::vector<const VertexBuffer*>& value);
	void SetVertexBuffer(const VertexBuffer& value);
	void SetVertexBuffer(const VertexBuffer** vertex_buffer, size_t count);
	void SetVertexBuffer(const void* memory, size_t size, size_t stride);
	template<class T>
	void SetVertexBuffer(const T* memory, size_t count)
	{
		SetVertexBuffer(memory, count * sizeof(T), sizeof(T));
	}
	template<class T>
	void SetVertexBuffer(const std::vector<T>& values)
	{
		SetVertexBuffer(values.data(), values.size());
	}

	void SetIndexBuffer(IndexBufferHandle* handle);
	void SetIndexBuffer(IndexBuffer& value);
	void SetIndexBuffer(const void* memory, size_t size, size_t stride);
	template<class T>
	void SetIndexBuffer(const T* memory, size_t count)
	{
		SetIndexBuffer(memory, count * sizeof(T), sizeof(T));
	}

	template<class T>
	void SetIndexBuffer(const std::vector<T>& values)
	{
		SetIndexBuffer(values.data(), values.size());
	}

	void SetUniformBuffer(uint32_t binding, UniformBufferHandle* handle);
	void SetUniformBuffer(uint32_t binding, UniformBuffer& value);
	void SetUniformBuffer(uint32_t binding, const void* memory, size_t size);
	template <class T>
	void SetUniformBuffer(uint32_t binding, const T& value)
	{
		SetUniformBuffer(binding, &const_cast<T&>(value), sizeof(T));
	}

	void SetBlendMode(const std::optional<BlendMode>& blend_mode);
	void SetDepthMode(const std::optional<DepthMode>& depth_mode);
	void SetStencilMode(const std::optional<StencilMode>& stencil_mode);
	void SetCullMode(CullMode cull_mode);
	void SetSampler(Sampler value);
	void SetTextureAddress(TextureAddress value);
	void SetFrontFace(FrontFace value);
	void SetDepthBias(const std::optional<DepthBias> depth_bias);

	void Clear(
		const std::optional<glm::vec4>& color = glm::vec4{ 0.0f, 0.0f, 0.0f, 1.0f }, 
		const std::optional<float>& depth = 1.0f, 
		const std::optional<uint8_t>& stencil = 0);
	void Draw(uint32_t vertex_count, uint32_t vertex_offset = 0, uint32_t instance_count = 1);
	void DrawIndexed(uint32_t index_count, uint32_t index_offset = 0, uint32_t instance_count = 1);

	void ReadPixels(const glm::i32vec2& pos, const glm::i32vec2& size, TextureHandle* dst_texture);
	void ReadPixels(const glm::i32vec2& pos, const glm::i32vec2& size, Texture& dst_texture);

	void Present();

	uint32_t GetWidth();
	uint32_t GetHeight();

	uint32_t GetBackbufferWidth();
	uint32_t GetBackbufferHeight();
	PixelFormat GetBackbufferFormat();

	RenderTarget* AcquireTransientRenderTarget(PixelFormat format = PixelFormat::RGBA32Float);
	RenderTarget* AcquireTransientRenderTarget(uint32_t width, uint32_t height, PixelFormat format = PixelFormat::RGBA32Float);
	void ReleaseTransientRenderTarget(RenderTarget* target);
	void DestroyTransientRenderTargets();

private:
	bool createAPI(const WindowPrivateData& data, const RenderSystemCreateInfo& createInfo);
	void destroyAPI();
	void resize(uint32_t width, uint32_t height);
	void present();
};

extern RenderSystem* gRenderSystem;