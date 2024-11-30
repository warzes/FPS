#include "stdafx.h"
#if RENDER_OPENGL
#include "RHISystem.h"
#include "ContextOpenGL.h"
//=============================================================================
TextureHandle* RHISystem::CreateTexture(uint32_t width, uint32_t height, PixelFormat format, uint32_t mip_count)
{
	auto texture = new TextureGL(width, height, format, mip_count);
	return (TextureHandle*)texture;
}
//=============================================================================
void RHISystem::WriteTexturePixels(TextureHandle* handle, uint32_t width, uint32_t height, PixelFormat format, const void* memory, uint32_t mip_level, uint32_t offset_x, uint32_t offset_y)
{
	auto texture = (TextureGL*)handle;
	texture->Write(width, height, format, memory, mip_level, offset_x, offset_y);
}
//=============================================================================
void RHISystem::GenerateMips(TextureHandle* handle)
{
	auto texture = (TextureGL*)handle;
	texture->GenerateMips();
}
//=============================================================================
void RHISystem::DestroyTexture(TextureHandle* handle)
{
	auto texture = (TextureGL*)handle;
	delete texture;
}
//=============================================================================
RenderTargetHandle* RHISystem::CreateRenderTarget(uint32_t width, uint32_t height, TextureHandle* texture_handle)
{
	auto texture = (TextureGL*)texture_handle;
	auto render_target = new RenderTargetGL(texture);
	return (RenderTargetHandle*)render_target;
}
//=============================================================================
void RHISystem::DestroyRenderTarget(RenderTargetHandle* handle)
{
	auto render_target = (RenderTargetGL*)handle;
	delete render_target;
}
//=============================================================================
ShaderHandle* RHISystem::CreateShader(const std::string& vertex_code, const std::string& fragment_code, const std::vector<std::string>& defines)
{
	auto shader = new ShaderGL(vertex_code, fragment_code, defines);
	return (ShaderHandle*)shader;
}
//=============================================================================
void RHISystem::DestroyShader(ShaderHandle* handle)
{
	auto shader = (ShaderGL*)handle;
	delete shader;
}
//=============================================================================
VertexBufferHandle* RHISystem::CreateVertexBuffer(size_t size, size_t stride)
{
	auto buffer = new VertexBufferGL(size, stride);
	return (VertexBufferHandle*)buffer;
}
//=============================================================================
void RHISystem::DestroyVertexBuffer(VertexBufferHandle* handle)
{
	auto buffer = (VertexBufferGL*)handle;
	delete buffer;
}
//=============================================================================
void RHISystem::WriteVertexBufferMemory(VertexBufferHandle* handle, const void* memory, size_t size, size_t stride)
{
	auto buffer = (VertexBufferGL*)handle;
	buffer->Write(memory, size);
	buffer->SetStride(stride);
}
//=============================================================================
IndexBufferHandle* RHISystem::CreateIndexBuffer(size_t size, size_t stride)
{
	auto buffer = new IndexBufferGL(size, stride);
	return (IndexBufferHandle*)buffer;
}
//=============================================================================
void RHISystem::DestroyIndexBuffer(IndexBufferHandle* handle)
{
	auto buffer = (IndexBufferGL*)handle;

	if (gContext.index_buffer == buffer)
		gContext.index_buffer = nullptr;

	delete buffer;
}
//=============================================================================
void RHISystem::WriteIndexBufferMemory(IndexBufferHandle* handle, const void* memory, size_t size, size_t stride)
{
	auto buffer = (IndexBufferGL*)handle;
	buffer->Write(memory, size);
	buffer->SetStride(stride);
}
//=============================================================================
UniformBufferHandle* RHISystem::CreateUniformBuffer(size_t size)
{
	auto buffer = new UniformBufferGL(size);
	return (UniformBufferHandle*)buffer;
}
//=============================================================================
void RHISystem::DestroyUniformBuffer(UniformBufferHandle* handle)
{
	auto buffer = (UniformBufferGL*)handle;
	delete buffer;
}
//=============================================================================
void RHISystem::WriteUniformBufferMemory(UniformBufferHandle* handle, const void* memory, size_t size)
{
	auto buffer = (UniformBufferGL*)handle;
	buffer->Write(memory, size);
}
//=============================================================================
#endif // RENDER_OPENGL