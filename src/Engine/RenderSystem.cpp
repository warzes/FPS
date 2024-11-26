#include "stdafx.h"
#include "RenderSystem.h"
//=============================================================================
RenderSystem* gRenderSystem{ nullptr };
glm::u32vec2 gSize = { 0, 0 };
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
bool RenderSystem::Create(const WindowPrivateData& data, const RenderSystemCreateInfo& createInfo)
{
	gRenderSystem = this;
	if (!createAPI(data, createInfo)) return false;

	gSize = { data.width, data.height };
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

	destroyAPI();
	gRenderSystem = nullptr;
}
//=============================================================================
void RenderSystem::Resize(uint32_t width, uint32_t height)
{
	resize(width, height);
	gSize = { width, height };
}
//=============================================================================
void RenderSystem::Present()
{
	present();
	DestroyTransientRenderTargets();
}
//=============================================================================
void RenderSystem::SetTexture(uint32_t binding, Texture& texture)
{
	SetTexture(binding, (TextureHandle*)texture);
}
//=============================================================================
void RenderSystem::SetRenderTarget(const std::vector<const RenderTarget*>& value)
{
	SetRenderTarget((const RenderTarget**)value.data(), value.size());
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
void RenderSystem::SetRenderTarget(std::nullopt_t)
{
	SetRenderTarget({});
}
//=============================================================================
void RenderSystem::SetShader(Shader& shader)
{
	SetShader((ShaderHandle*)shader);
}
//=============================================================================
void RenderSystem::SetInputLayout(const InputLayout& value)
{
	SetInputLayout(std::vector<InputLayout>{ value });
}
//=============================================================================
void RenderSystem::SetVertexBuffer(const std::vector<const VertexBuffer*>& vertex_buffers)
{
	SetVertexBuffer((const VertexBuffer**)vertex_buffers.data(), vertex_buffers.size());
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
void RenderSystem::SetIndexBuffer(IndexBuffer& value)
{
	SetIndexBuffer((IndexBufferHandle*)value);
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
	SetUniformBuffer(binding, (UniformBufferHandle*)&value);
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
void RenderSystem::ReadPixels(const glm::i32vec2& pos, const glm::i32vec2& size, Texture& dst_texture)
{
	ReadPixels(pos, size, (TextureHandle*)dst_texture);
}
//=============================================================================
uint32_t RenderSystem::GetWidth()
{
	return gSize.x;
}
//=============================================================================
uint32_t RenderSystem::GetHeight()
{
	return gSize.y;
}
//=============================================================================
uint32_t RenderSystem::GetBackbufferWidth()
{
	return gRenderTargetSize.value_or(gSize).x;
}
//=============================================================================
uint32_t RenderSystem::GetBackbufferHeight()
{
	return gRenderTargetSize.value_or(gSize).y;
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