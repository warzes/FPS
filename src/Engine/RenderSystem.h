#pragma once

#include "WindowStructs.h"
#include "RHIResources.h"

class RenderSystem final
{
public:
	~RenderSystem();

	bool Create(const WindowData& data, const RenderSystemCreateInfo& createInfo);
	void Destroy();

	void Resize(uint32_t width, uint32_t height);

	void Present();

	void Clear(
		const std::optional<glm::vec4>& color = glm::vec4{ 0.2f, 0.4f, 0.6f, 1.0f }, 
		const std::optional<float>&     depth = 1.0f, 
		const std::optional<uint8_t>&   stencil = 0);
	void Draw(uint32_t vertexCount, uint32_t vertexOffset = 0, uint32_t instanceCount = 1);
	void DrawIndexed(uint32_t indexCount, uint32_t indexOffset = 0, uint32_t instanceCount = 1);
	void ReadPixels(const glm::i32vec2& pos, const glm::i32vec2& size, Texture2D& dstTexture);

	void SetTopology(Topology topology);
	void SetViewport(const std::optional<Viewport>& viewport);
	void SetScissor(const std::optional<Scissor>& scissor);

	void SetBlendMode(const std::optional<BlendMode>& blendMode);
	void SetDepthMode(const std::optional<DepthMode>& depthMode);
	void SetStencilMode(const std::optional<StencilMode>& stencilMode);
	void SetCullMode(CullMode cullMode);
	void SetSampler(Sampler value);
	void SetTextureAddress(TextureAddress value);
	void SetFrontFace(FrontFace value);
	void SetDepthBias(const std::optional<DepthBias> depthBias);

	void SetShader(const Shader& shader);
	void SetInputLayout(const InputLayout& value);
	void SetInputLayout(const std::vector<InputLayout>& value);

	void SetTexture(uint32_t binding, const Texture2D& texture);
	void SetRenderTarget(const std::vector<const RenderTarget*>& value);
	void SetRenderTarget(const RenderTarget& value);
	void SetRenderTarget(std::nullopt_t value);

	void SetVertexBuffer(const std::vector<const VertexBuffer*>& value);
	void SetVertexBuffer(const VertexBuffer& value);
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

	void SetIndexBuffer(const IndexBuffer& value);
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

	void SetUniformBuffer(uint32_t binding, const UniformBuffer& value);
	void SetUniformBuffer(uint32_t binding, const void* memory, size_t size);
	template <class T>
	void SetUniformBuffer(uint32_t binding, const T& value)
	{
		SetUniformBuffer(binding, &const_cast<T&>(value), sizeof(T));
	}
	
#if RENDER_VULKAN
	void DispatchRays(uint32_t width, uint32_t height, uint32_t depth);

	void SetShader(const RaytracingShader& shader);
	void SetStorageBuffer(uint32_t binding, const StorageBuffer& value);
	void SetStorageBuffer(uint32_t binding, const void* memory, size_t size);
	void SetAccelerationStructure(uint32_t binding, const TopLevelAccelerationStructure& value);
#endif

	uint32_t GetWidth();
	uint32_t GetHeight();

	uint32_t GetBackBufferWidth();
	uint32_t GetBackBufferHeight();
	PixelFormat GetBackBufferFormat();

	RenderTarget* AcquireTransientRenderTarget(PixelFormat format = PixelFormat::RGBA32Float);
	RenderTarget* AcquireTransientRenderTarget(uint32_t width, uint32_t height, PixelFormat format = PixelFormat::RGBA32Float);
	void ReleaseTransientRenderTarget(RenderTarget* target);
	void DestroyTransientRenderTargets();

private:
	glm::u32vec2                                m_frameSize = { 0, 0 };
	std::optional<glm::u32vec2>                 m_renderTargetSize;
	PixelFormat                                 m_backBufferFormat;

	std::optional<VertexBuffer>                 m_vertexBuffer;
	std::optional<IndexBuffer>                  m_indexBuffer;
	std::unordered_map<uint32_t, UniformBuffer> m_uniformBuffers;
	std::unordered_map<TransientRenderTargetDesc, std::unordered_set<std::shared_ptr<TransientRenderTarget>>> m_transientRenderTargets;
#if RENDER_VULKAN
	std::unordered_map<uint32_t, StorageBuffer> m_storageBuffers;
#endif
};

extern RenderSystem* gRenderSystem;