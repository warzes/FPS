#include "stdafx.h"
#if RENDER_D3D11
#include "GAPISystem.h"
#include "ContextD3D11.h"
#include "TextureD3D11.h"
#include "RenderTargetD3D11.h"
#include "BufferD3D11.h"
#include "ShaderD3D11.h"
#include "Log.h"
//=============================================================================
TextureHandle* GAPISystem::CreateTexture(uint32_t width, uint32_t height, PixelFormat format, uint32_t mip_count)
{
	auto texture = new TextureD3D11(width, height, format, mip_count);
	return (TextureHandle*)texture;
}
//=============================================================================
void GAPISystem::WriteTexturePixels(TextureHandle* handle, uint32_t width, uint32_t height, PixelFormat format, const void* memory, uint32_t mip_level, uint32_t offset_x, uint32_t offset_y)
{
	auto texture = (TextureD3D11*)handle;
	texture->Write(width, height, format, memory, mip_level, offset_x, offset_y);
}
//=============================================================================
void GAPISystem::GenerateMips(TextureHandle* handle)
{
	auto texture = (TextureD3D11*)handle;
	texture->GenerateMips();
}
//=============================================================================
void GAPISystem::DestroyTexture(TextureHandle* handle)
{
	auto texture = (TextureD3D11*)handle;
	delete texture;
}
//=============================================================================
RenderTargetHandle* GAPISystem::CreateRenderTarget(uint32_t width, uint32_t height, TextureHandle* texture_handle)
{
	auto texture = (TextureD3D11*)texture_handle;
	auto render_target = new RenderTargetD3D11(width, height, texture);
	return (RenderTargetHandle*)render_target;
}
//=============================================================================
void GAPISystem::DestroyRenderTarget(RenderTargetHandle* handle)
{
	auto render_target = (RenderTargetD3D11*)handle;
	delete render_target;
}
//=============================================================================
ShaderHandle* GAPISystem::CreateShader(const std::string& vertex_code, const std::string& fragment_code, const std::vector<std::string>& defines)
{
	auto shader = new ShaderD3D11(vertex_code, fragment_code, defines);
	return (ShaderHandle*)shader;
}
//=============================================================================
void GAPISystem::DestroyShader(ShaderHandle* handle)
{
	auto shader = (ShaderD3D11*)handle;
	delete shader;
}
//=============================================================================
VertexBufferHandle* GAPISystem::CreateVertexBuffer(size_t size, size_t stride)
{
	auto buffer = new VertexBufferD3D11(size, stride);
	return (VertexBufferHandle*)buffer;
}
//=============================================================================
void GAPISystem::DestroyVertexBuffer(VertexBufferHandle* handle)
{
	auto buffer = (VertexBufferD3D11*)handle;
	delete buffer;
}
//=============================================================================
void GAPISystem::WriteVertexBufferMemory(VertexBufferHandle* handle, const void* memory, size_t size, size_t stride)
{
	auto buffer = (VertexBufferD3D11*)handle;
	buffer->Write(memory, size);
	buffer->SetStride(stride);
}
//=============================================================================
IndexBufferHandle* GAPISystem::CreateIndexBuffer(size_t size, size_t stride)
{
	auto buffer = new IndexBufferD3D11(size, stride);
	return (IndexBufferHandle*)buffer;
}
//=============================================================================
void GAPISystem::DestroyIndexBuffer(IndexBufferHandle* handle)
{
	auto buffer = (IndexBufferD3D11*)handle;
	delete buffer;
}
//=============================================================================
void GAPISystem::WriteIndexBufferMemory(IndexBufferHandle* handle, const void* memory, size_t size, size_t stride)
{
	auto buffer = (IndexBufferD3D11*)handle;
	buffer->Write(memory, size);
	buffer->SetStride(stride);
}
//=============================================================================
UniformBufferHandle* GAPISystem::CreateUniformBuffer(size_t size)
{
	auto buffer = new UniformBufferD3D11(size);
	return (UniformBufferHandle*)buffer;
}
//=============================================================================
void GAPISystem::DestroyUniformBuffer(UniformBufferHandle* handle)
{
	auto buffer = (UniformBufferD3D11*)handle;
	delete buffer;
}
//=============================================================================
void GAPISystem::WriteUniformBufferMemory(UniformBufferHandle* handle, const void* memory, size_t size)
{
	auto buffer = (UniformBufferD3D11*)handle;
	buffer->Write(memory, size);
}
//=============================================================================
#endif // RENDER_D3D11