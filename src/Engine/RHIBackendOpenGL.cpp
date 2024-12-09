#include "stdafx.h"
#if RENDER_OPENGL
#include "RHIBackend.h"
#include "ContextD3D12.h"
#include "ShaderD3D12.h"
#include "TextureD3D12.h"
#include "RenderTargetD3D12.h"
#include "BufferD3D12.h"
#include "Log.h"
#include "RenderResources.h"
//=============================================================================
RenderContext gContext{};
//=============================================================================
bool RHIBackend::CreateAPI(const WindowData& data, const RenderSystemCreateInfo& createInfo)
{

}
//=============================================================================
void RHIBackend::DestroyAPI()
{
}
//=============================================================================
void RHIBackend::ResizeFrameBuffer(uint32_t width, uint32_t height)
{
	
}
//=============================================================================
void RHIBackend::Present()
{
}
//=============================================================================
void RHIBackend::Clear(const std::optional<glm::vec4>& color, const std::optional<float>& depth, const std::optional<uint8_t>& stencil)
{
}
//=============================================================================
void RHIBackend::Draw(uint32_t vertexCount, uint32_t vertexOffset, uint32_t instanceCount)
{
}
//=============================================================================
void RHIBackend::DrawIndexed(uint32_t indexCount, uint32_t indexOffset, uint32_t instanceCount)
{
}
//=============================================================================
void RHIBackend::ReadPixels(const glm::i32vec2& pos, const glm::i32vec2& size, TextureHandle* dstTextureHandle)
{
}
//=============================================================================
ShaderHandle* RHIBackend::CreateShader(const std::string& vertexCode, const std::string& fragmentCode, const std::vector<std::string>& defines)
{
}
//=============================================================================
void RHIBackend::DestroyShader(ShaderHandle* handle)
{
}
//=============================================================================
TextureHandle* RHIBackend::CreateTexture(uint32_t width, uint32_t height, PixelFormat format, uint32_t mip_count)
{
}
//=============================================================================
void RHIBackend::WriteTexturePixels(TextureHandle* handle, uint32_t width, uint32_t height, PixelFormat format, const void* memory, uint32_t mip_level, uint32_t offset_x, uint32_t offset_y)
{
}
//=============================================================================
void RHIBackend::GenerateMips(TextureHandle* handle)
{
}
//=============================================================================
void RHIBackend::DestroyTexture(TextureHandle* handle)
{

}
//=============================================================================
RenderTargetHandle* RHIBackend::CreateRenderTarget(uint32_t width, uint32_t height, TextureHandle* texture_handle)
{
}
//=============================================================================
void RHIBackend::DestroyRenderTarget(RenderTargetHandle* handle)
{

}
//=============================================================================
VertexBufferHandle* RHIBackend::CreateVertexBuffer(size_t size, size_t stride)
{
}
//=============================================================================
void RHIBackend::DestroyVertexBuffer(VertexBufferHandle* handle)
{

}
//=============================================================================
void RHIBackend::WriteVertexBufferMemory(VertexBufferHandle* handle, const void* memory, size_t size, size_t stride)
{
}
//=============================================================================
IndexBufferHandle* RHIBackend::CreateIndexBuffer(size_t size, size_t stride)
{
}
//=============================================================================
void RHIBackend::DestroyIndexBuffer(IndexBufferHandle* handle)
{
}
//=============================================================================
void RHIBackend::WriteIndexBufferMemory(IndexBufferHandle* handle, const void* memory, size_t size, size_t stride)
{
}
//=============================================================================
UniformBufferHandle* RHIBackend::CreateUniformBuffer(size_t size)
{
}
//=============================================================================
void RHIBackend::DestroyUniformBuffer(UniformBufferHandle* handle)
{

}
//=============================================================================
void RHIBackend::WriteUniformBufferMemory(UniformBufferHandle* handle, const void* memory, size_t size)
{

}
//=============================================================================
void RHIBackend::SetTopology(Topology topology)
{

}
//=============================================================================
void RHIBackend::SetViewport(std::optional<Viewport> viewport)
{

}
//=============================================================================
void RHIBackend::SetScissor(std::optional<Scissor> scissor)
{

}
//=============================================================================
void RHIBackend::SetBlendMode(const std::optional<BlendMode>& blend_mode)
{

}
//=============================================================================
void RHIBackend::SetDepthMode(const std::optional<DepthMode>& depth_mode)
{

}
//=============================================================================
void RHIBackend::SetStencilMode(const std::optional<StencilMode>& stencil_mode)
{

}
//=============================================================================
void RHIBackend::SetCullMode(CullMode cull_mode)
{

}
//=============================================================================
void RHIBackend::SetSampler(Sampler value)
{

}
//=============================================================================
void RHIBackend::SetTextureAddress(TextureAddress value)
{

}
//=============================================================================
void RHIBackend::SetFrontFace(FrontFace value)
{

}
//=============================================================================
void RHIBackend::SetDepthBias(const std::optional<DepthBias> depth_bias)
{

}
//=============================================================================
void RHIBackend::SetShader(ShaderHandle* handle)
{

}
//=============================================================================
void RHIBackend::SetInputLayout(const std::vector<InputLayout>& value)
{

}
//=============================================================================
void RHIBackend::SetTexture(uint32_t binding, TextureHandle* handle)
{

}
//=============================================================================
void RHIBackend::SetRenderTarget(const RenderTarget** render_target, size_t count)
{

}
//=============================================================================
void RHIBackend::SetVertexBuffer(const VertexBuffer** vertex_buffer, size_t count)
{

}
//=============================================================================
void RHIBackend::SetIndexBuffer(IndexBufferHandle* handle)
{
}
//=============================================================================
void RHIBackend::SetUniformBuffer(uint32_t binding, UniformBufferHandle* handle)
{
}
//=============================================================================
#endif // RENDER_OPENGL