#include "stdafx.h"
#include "RenderSystem.h"
#include "RHIBackend.h"
#include "RenderUtils.h"
//=============================================================================
#if PLATFORM_WINDOWS
extern "C" {
	_declspec(dllexport) uint32_t NvOptimusEnablement = 1;
	_declspec(dllexport) uint32_t AmdPowerXpressRequestHighPerformance = 1;
}
#endif // PLATFORM_WINDOWS
//=============================================================================
RenderSystem* gRenderSystem{ nullptr };
//=============================================================================
RenderSystem::~RenderSystem()
{
	assert(!gRenderSystem);
}
//=============================================================================
bool RenderSystem::Create(const WindowData& data, const RenderSystemCreateInfo& createInfo)
{
	if (!RHIBackend::CreateAPI(data, createInfo)) return false;

	m_frameSize = { data.width, data.height };
	m_renderTargetSize.reset();
	m_backBufferFormat = PixelFormat::RGBA8UNorm;

	gRenderSystem = this;

	return true;
}
//=============================================================================
void RenderSystem::Destroy()
{
	utils::ClearContext();
	m_indexBuffer.reset();
	m_vertexBuffer.reset();
	m_uniformBuffers.clear();
#if RENDER_VULKAN
	m_storageBuffers.clear();
#endif
	RHIBackend::DestroyAPI();
	gRenderSystem = nullptr;
}
//=============================================================================
void RenderSystem::Resize(uint32_t width, uint32_t height)
{
	if (m_frameSize.x != width || m_frameSize.y != height)
	{
		RHIBackend::ResizeFrameBuffer(width, height);
		m_frameSize = { width, height };
	}
}
//=============================================================================
void RenderSystem::Present()
{
	RHIBackend::Present();
	DestroyTransientRenderTargets();
}
//=============================================================================
void RenderSystem::Clear(const std::optional<glm::vec4>& color, const std::optional<float>& depth, const std::optional<uint8_t>& stencil)
{
	RHIBackend::Clear(color, depth, stencil);
}
//=============================================================================
void RenderSystem::Draw(uint32_t vertex_count, uint32_t vertex_offset, uint32_t instance_count)
{
	RHIBackend::Draw(vertex_count, vertex_offset, instance_count);
}
//=============================================================================
void RenderSystem::DrawIndexed(uint32_t index_count, uint32_t index_offset, uint32_t instance_count)
{
	RHIBackend::DrawIndexed(index_count, index_offset, instance_count);
}
//=============================================================================
void RenderSystem::ReadPixels(const glm::i32vec2& pos, const glm::i32vec2& size, Texture& dst_texture)
{
	RHIBackend::ReadPixels(pos, size, dst_texture);
}
//=============================================================================
void RenderSystem::SetTopology(Topology topology)
{
	RHIBackend::SetTopology(topology);
}
//=============================================================================
void RenderSystem::SetViewport(const std::optional<Viewport>& viewport)
{
	RHIBackend::SetViewport(viewport);
}
//=============================================================================
void RenderSystem::SetScissor(const std::optional<Scissor>& scissor)
{
	RHIBackend::SetScissor(scissor);
}
//=============================================================================
void RenderSystem::SetBlendMode(const std::optional<BlendMode>& blend_mode)
{
	RHIBackend::SetBlendMode(blend_mode);
}
//=============================================================================
void RenderSystem::SetDepthMode(const std::optional<DepthMode>& depth_mode)
{
	RHIBackend::SetDepthMode(depth_mode);
}
//=============================================================================
void RenderSystem::SetStencilMode(const std::optional<StencilMode>& stencil_mode)
{
	RHIBackend::SetStencilMode(stencil_mode);
}
//=============================================================================
void RenderSystem::SetCullMode(CullMode cull_mode)
{
	RHIBackend::SetCullMode(cull_mode);
}
//=============================================================================
void RenderSystem::SetSampler(Sampler value)
{
	RHIBackend::SetSampler(value);
}
//=============================================================================
void RenderSystem::SetTextureAddress(TextureAddress value)
{
	RHIBackend::SetTextureAddress(value);
}
//=============================================================================
void RenderSystem::SetFrontFace(FrontFace value)
{
	RHIBackend::SetFrontFace(value);
}
//=============================================================================
void RenderSystem::SetDepthBias(const std::optional<DepthBias> depth_bias)
{
	RHIBackend::SetDepthBias(depth_bias);
}
//=============================================================================
void RenderSystem::SetShader(const Shader& shader)
{
	RHIBackend::SetShader(const_cast<Shader&>(shader));
}
//=============================================================================
void RenderSystem::SetInputLayout(const InputLayout& value)
{
	RHIBackend::SetInputLayout({ value });
}
//=============================================================================
void RenderSystem::SetInputLayout(const std::vector<InputLayout>& value)
{
	RHIBackend::SetInputLayout(value);
}
//=============================================================================
void RenderSystem::SetTexture(uint32_t binding, const Texture& texture)
{
	RHIBackend::SetTexture(binding, const_cast<Texture&>(texture));
}
//=============================================================================
void RenderSystem::SetRenderTarget(const std::vector<const RenderTarget*>& value)
{
	RHIBackend::SetRenderTarget((const RenderTarget**)value.data(), value.size());
	if (value.empty())
	{
		m_renderTargetSize.reset();
		m_backBufferFormat = PixelFormat::RGBA8UNorm;
	}
	else
	{
		m_renderTargetSize = { value.at(0)->GetWidth(), value.at(0)->GetHeight() };
		m_backBufferFormat = value.at(0)->GetFormat();
	}
}
//=============================================================================
void RenderSystem::SetRenderTarget(const RenderTarget& value)
{
	SetRenderTarget({ (RenderTarget*)&value });
}
//=============================================================================
void RenderSystem::SetRenderTarget(std::nullopt_t value)
{
	SetRenderTarget({});
}
//=============================================================================
void RenderSystem::SetVertexBuffer(const std::vector<const VertexBuffer*>& value)
{
	RHIBackend::SetVertexBuffer((const VertexBuffer**)value.data(), value.size());
}
//=============================================================================
void RenderSystem::SetVertexBuffer(const VertexBuffer& value)
{
	SetVertexBuffer({ &value });
}
//=============================================================================
void RenderSystem::SetVertexBuffer(const void* memory, size_t size, size_t stride)
{
	assert(size > 0);

	size_t vertex_buffer_size = 0;

	if (m_vertexBuffer.has_value())
		vertex_buffer_size = m_vertexBuffer->GetSize();

	if (vertex_buffer_size < size)
		m_vertexBuffer.emplace(memory, size, stride);
	else
		m_vertexBuffer.value().Write(memory, size, stride);

	SetVertexBuffer(m_vertexBuffer.value());
}
//=============================================================================
void RenderSystem::SetIndexBuffer(const IndexBuffer& value)
{
	RHIBackend::SetIndexBuffer(const_cast<IndexBuffer&>(value));
}
//=============================================================================
void RenderSystem::SetIndexBuffer(const void* memory, size_t size, size_t stride)
{
	assert(size > 0);

	size_t index_buffer_size = 0;

	if (m_indexBuffer.has_value())
		index_buffer_size = m_indexBuffer->GetSize();

	if (index_buffer_size < size)
		m_indexBuffer.emplace(memory, size, stride);
	else
		m_indexBuffer.value().Write(memory, size, stride);

	SetIndexBuffer(m_indexBuffer.value());
}
//=============================================================================
void RenderSystem::SetUniformBuffer(uint32_t binding, const UniformBuffer& value)
{
	RHIBackend::SetUniformBuffer(binding, const_cast<UniformBuffer&>(value));
}
//=============================================================================
void RenderSystem::SetUniformBuffer(uint32_t binding, const void* memory, size_t size)
{
	assert(size > 0);

	if (!m_uniformBuffers.contains(binding))
		m_uniformBuffers.emplace(binding, size);

	auto& buffer = m_uniformBuffers.at(binding);

	if (buffer.GetSize() < size)
		buffer = UniformBuffer(size);

	buffer.Write(memory, size);

	SetUniformBuffer(binding, buffer);
}
//=============================================================================
#if RENDER_VULKAN
void RenderSystem::DispatchRays(uint32_t width, uint32_t height, uint32_t depth)
{
	RHIBackend::DispatchRays(width, height, depth);
}
#endif
//=============================================================================
#if RENDER_VULKAN
void RenderSystem::SetShader(const RaytracingShader& shader)
{
	RHIBackend::SetRaytracingShader(const_cast<RaytracingShader&>(shader));
}
#endif
//=============================================================================
#if RENDER_VULKAN
void RenderSystem::SetStorageBuffer(uint32_t binding, const StorageBuffer& value)
{
	RHIBackend::SetStorageBuffer(binding, const_cast<StorageBuffer&>(value));
}
#endif
//=============================================================================
#if RENDER_VULKAN
void RenderSystem::SetStorageBuffer(uint32_t binding, const void* memory, size_t size)
{
	assert(size > 0);

	if (!m_storageBuffers.contains(binding))
		m_storageBuffers.emplace(binding, size);

	auto& buffer = m_storageBuffers.at(binding);

	if (buffer.GetSize() < size)
		buffer = StorageBuffer(size);

	buffer.Write(memory, size);

	SetStorageBuffer(binding, buffer);
}
#endif
//=============================================================================
#if RENDER_VULKAN
void RenderSystem::SetAccelerationStructure(uint32_t binding, const TopLevelAccelerationStructure& value)
{
	RHIBackend::SetAccelerationStructure(binding, const_cast<TopLevelAccelerationStructure&>(value));
}
#endif
//=============================================================================
uint32_t RenderSystem::GetWidth()
{
	return m_frameSize.x;
}
//=============================================================================
uint32_t RenderSystem::GetHeight()
{
	return m_frameSize.y;
}
//=============================================================================
uint32_t RenderSystem::GetBackBufferWidth()
{
	return m_renderTargetSize.value_or(m_frameSize).x;
}
//=============================================================================
uint32_t RenderSystem::GetBackBufferHeight()
{
	return m_renderTargetSize.value_or(m_frameSize).y;
}
//=============================================================================
PixelFormat RenderSystem::GetBackBufferFormat()
{
	return m_backBufferFormat;
}
//=============================================================================
RenderTarget* RenderSystem::AcquireTransientRenderTarget(PixelFormat format)
{
	return AcquireTransientRenderTarget(GetBackBufferWidth(), GetBackBufferHeight(), format);
}
//=============================================================================
RenderTarget* RenderSystem::AcquireTransientRenderTarget(uint32_t width, uint32_t height, PixelFormat format)
{
	auto desc = TransientRenderTargetDesc(width, height, format);

	for (auto& [_desc, transient_rts] : m_transientRenderTargets)
	{
		if (desc != _desc)
			continue;

		for (auto& transient_rt : transient_rts)
		{
			if (transient_rt->GetState() == TransientRenderTarget::State::Active)
				continue;

			transient_rt->SetState(TransientRenderTarget::State::Active);
			return transient_rt.get();
		}
	}

	auto transient_rt = std::make_shared<TransientRenderTarget>(width, height, format);
	m_transientRenderTargets[desc].insert(transient_rt);
	return transient_rt.get();
}
//=============================================================================
void RenderSystem::ReleaseTransientRenderTarget(RenderTarget* target)
{
	for (auto& [desc, transient_rts] : m_transientRenderTargets)
	{
		for (auto& transient_rt : transient_rts)
		{
			if (transient_rt.get() != target)
				continue;

			transient_rt->SetState(TransientRenderTarget::State::Inactive);
			return;
		}
	}
	assert(false);
}
//=============================================================================
void RenderSystem::DestroyTransientRenderTargets()
{
	for (auto& [desc, transient_rts] : m_transientRenderTargets)
	{
		std::erase_if(transient_rts, [](const std::shared_ptr<TransientRenderTarget>& transient_rt) {
			return transient_rt->GetState() == TransientRenderTarget::State::Destroy;
			});

		for (auto& transient_rt : transient_rts)
		{
			transient_rt->SetState(TransientRenderTarget::State::Destroy);
		}
	}

	std::erase_if(m_transientRenderTargets, [](const auto& pair) {
		return pair.second.empty();
		});
}
//=============================================================================