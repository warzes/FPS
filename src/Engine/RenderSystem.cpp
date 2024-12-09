#include "stdafx.h"
#include "RenderSystem.h"
#include "RHIBackend.h"
//=============================================================================
RenderSystem* gRenderSystem{ nullptr };
std::optional<glm::u32vec2> gRenderTargetSize;
PixelFormat gBackbufferFormat;
std::optional<VertexBuffer> gVertexBuffer;
std::optional<IndexBuffer> gIndexBuffer;
std::unordered_map<uint32_t, UniformBuffer> gUniformBuffers;
std::unordered_map<TransientRenderTargetDesc, std::unordered_set<std::shared_ptr<TransientRenderTarget>>> gTransientRenderTargets;
//=============================================================================
RenderSystem::~RenderSystem()
{
	assert(!gRenderSystem);
}
//=============================================================================
bool RenderSystem::Create(const WindowData& data, const RenderSystemCreateInfo& createInfo)
{
	gRenderSystem = this;
	if (!RHIBackend::CreateAPI(data, createInfo)) return false;

	m_frameSize = { data.width, data.height };
	gRenderTargetSize.reset();
	gBackbufferFormat = PixelFormat::RGBA8UNorm;

	return true;
}
//=============================================================================
void RenderSystem::Destroy()
{
	gIndexBuffer.reset();
	gVertexBuffer.reset();
	gUniformBuffers.clear();

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
		gRenderTargetSize.reset();
		gBackbufferFormat = PixelFormat::RGBA8UNorm;
	}
	else
	{
		gRenderTargetSize = { value.at(0)->GetWidth(), value.at(0)->GetHeight() };
		gBackbufferFormat = value.at(0)->GetFormat();
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

	if (gVertexBuffer.has_value())
		vertex_buffer_size = gVertexBuffer->GetSize();

	if (vertex_buffer_size < size)
		gVertexBuffer.emplace(memory, size, stride);
	else
		gVertexBuffer.value().Write(memory, size, stride);

	SetVertexBuffer(gVertexBuffer.value());
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

	if (gIndexBuffer.has_value())
		index_buffer_size = gIndexBuffer->GetSize();

	if (index_buffer_size < size)
		gIndexBuffer.emplace(memory, size, stride);
	else
		gIndexBuffer.value().Write(memory, size, stride);

	SetIndexBuffer(gIndexBuffer.value());
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

	if (!gUniformBuffers.contains(binding))
		gUniformBuffers.emplace(binding, size);

	auto& buffer = gUniformBuffers.at(binding);

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

	if (!gStorageBuffers.contains(binding))
		gStorageBuffers.emplace(binding, size);

	auto& buffer = gStorageBuffers.at(binding);

	if (buffer.getSize() < size)
		buffer = StorageBuffer(size);

	buffer.write(memory, size);

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
uint32_t RenderSystem::GetBackbufferWidth()
{
	return gRenderTargetSize.value_or(m_frameSize).x;
}
//=============================================================================
uint32_t RenderSystem::GetBackbufferHeight()
{
	return gRenderTargetSize.value_or(m_frameSize).y;
}
//=============================================================================
PixelFormat RenderSystem::GetBackbufferFormat()
{
	return gBackbufferFormat;
}
//=============================================================================
RenderTarget* RenderSystem::AcquireTransientRenderTarget(PixelFormat format)
{
	return AcquireTransientRenderTarget(GetBackbufferWidth(), GetBackbufferHeight(), format);
}
//=============================================================================
RenderTarget* RenderSystem::AcquireTransientRenderTarget(uint32_t width, uint32_t height, PixelFormat format)
{
	auto desc = TransientRenderTargetDesc(width, height, format);

	for (auto& [_desc, transient_rts] : gTransientRenderTargets)
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
	gTransientRenderTargets[desc].insert(transient_rt);
	return transient_rt.get();
}
//=============================================================================
void RenderSystem::ReleaseTransientRenderTarget(RenderTarget* target)
{
	for (auto& [desc, transient_rts] : gTransientRenderTargets)
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
	for (auto& [desc, transient_rts] : gTransientRenderTargets)
	{
		std::erase_if(transient_rts, [](const std::shared_ptr<TransientRenderTarget>& transient_rt) {
			return transient_rt->GetState() == TransientRenderTarget::State::Destroy;
			});

		for (auto& transient_rt : transient_rts)
		{
			transient_rt->SetState(TransientRenderTarget::State::Destroy);
		}
	}

	std::erase_if(gTransientRenderTargets, [](const auto& pair) {
		return pair.second.empty();
		});
}
//=============================================================================