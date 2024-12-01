#pragma once

#include "WindowStructs.h"
#include "RenderResources.h"


class RenderSystem final
{
public:
	~RenderSystem();

	bool Create(const WindowData& data, const RenderSystemCreateInfo& createInfo);
	void Destroy();

	void Resize(uint32_t width, uint32_t height);

	void Present();

	void Clear(const std::optional<glm::vec4>& color = glm::vec4{ 0.0f, 0.0f, 0.0f, 1.0f }, const std::optional<float>& depth = 1.0f, const std::optional<uint8_t>& stencil = 0);
	void Draw(uint32_t vertex_count, uint32_t vertex_offset = 0, uint32_t instance_count = 1);
	void DrawIndexed(uint32_t index_count, uint32_t index_offset = 0, uint32_t instance_count = 1);

	void ReadPixels(const glm::i32vec2& pos, const glm::i32vec2& size, Texture& dst_texture);

	void SetTopology(Topology topology);
	void SetViewport(const std::optional<Viewport>& viewport);
	void SetScissor(const std::optional<Scissor>& scissor);

	void SetBlendMode(const std::optional<BlendMode>& blend_mode);
	void SetDepthMode(const std::optional<DepthMode>& depth_mode);
	void SetStencilMode(const std::optional<StencilMode>& stencil_mode);
	void SetCullMode(CullMode cull_mode);
	void SetSampler(Sampler value);
	void SetTextureAddress(TextureAddress value);
	void SetFrontFace(FrontFace value);
	void SetDepthBias(const std::optional<DepthBias> depth_bias);

	void SetShader(const Shader& shader);
	void SetInputLayout(const InputLayout& value);
	void SetInputLayout(const std::vector<InputLayout>& value);

	void SetTexture(uint32_t binding, const Texture& texture);
	void SetRenderTarget(const std::vector<const RenderTarget*>& value);
	void SetRenderTarget(const RenderTarget& value);
	void SetRenderTarget(std::nullopt_t value);

	void SetVertexBuffer(const std::vector<const VertexBuffer*>& value);
	void SetVertexBuffer(const VertexBuffer& value);
	void SetIndexBuffer(const IndexBuffer& value);
	void SetUniformBuffer(uint32_t binding, const UniformBuffer& value);

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

	void SetUniformBuffer(uint32_t binding, const void* memory, size_t size);

	template <class T>
	void SetUniformBuffer(uint32_t binding, const T& value)
	{
		SetUniformBuffer(binding, &const_cast<T&>(value), sizeof(T));
	}


#if RENDER_VULKAN
	void SetShader(const RaytracingShader& shader);
	void SetStorageBuffer(uint32_t binding, const StorageBuffer& value);
	void SetStorageBuffer(uint32_t binding, const void* memory, size_t size);
	void SetAccelerationStructure(uint32_t binding, const TopLevelAccelerationStructure& value);
	void DispatchRays(uint32_t width, uint32_t height, uint32_t depth);
#endif

















	
	void SetTopology(Topology topology);
	void SetViewport(std::optional<Viewport> viewport);
	void SetScissor(std::optional<Scissor> scissor);

	void SetShader(const Shader& shader);

	void SetTexture(uint32_t binding, TextureHandle* handle);
	void SetTexture(uint32_t binding, const TextureHandle* handle);
	void SetTexture(uint32_t binding, Texture& texture);

	void SetRenderTarget(std::nullopt_t);
	void SetRenderTarget(const RenderTarget& value);
	void SetRenderTarget(const RenderTarget** renderTarget, size_t count);
	void SetRenderTarget(const std::vector<const RenderTarget*>& value);


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
	void SetIndexBuffer(const IndexBufferHandle* handle);
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



	uint32_t GetWidth();
	uint32_t GetHeight();

	uint32_t GetBackbufferWidth();
	uint32_t GetBackbufferHeight();
	PixelFormat GetBackbufferFormat();

	RenderTarget* AcquireTransientRenderTarget(PixelFormat format = PixelFormat::RGBA32Float);
	RenderTarget* AcquireTransientRenderTarget(uint32_t width, uint32_t height, PixelFormat format = PixelFormat::RGBA32Float);
	void ReleaseTransientRenderTarget(RenderTarget* target);
	void DestroyTransientRenderTargets();

	// raytracing
#if RENDER_VULKAN
	void SetStorageBuffer(uint32_t binding, StorageBufferHandle* handle);
	void SetRaytracingShader(RaytracingShaderHandle* handle);
	void SetAccelerationStructure(uint32_t binding, TopLevelAccelerationStructureHandle* handle);

	void DispatchRays(uint32_t width, uint32_t height, uint32_t depth);	
#endif // RENDER_VULKAN

private:
	bool createAPI(const WindowData& data, const RenderSystemCreateInfo& createInfo);
	void destroyAPI();
	void resize(uint32_t width, uint32_t height);
	void present();



	glm::u32vec2 m_frameSize = { 0, 0 };
};

extern RenderSystem* gRenderSystem;