﻿#include "stdafx.h"
#if RENDER_D3D12
#include "RHISystem.h"
#include "ContextD3D12.h"
#include "BufferD3D12.h"
#include "RenderTargetD3D12.h"
#include "TextureD3D12.h"
#include "ShaderD3D12.h"
//=============================================================================
TextureHandle* RHISystem::CreateTexture(uint32_t width, uint32_t height, PixelFormat format, uint32_t mip_count)
{
	auto texture = new TextureD3D12(width, height, format, mip_count);
	return (TextureHandle*)texture;
}
//=============================================================================
void RHISystem::WriteTexturePixels(TextureHandle* handle, uint32_t width, uint32_t height, PixelFormat format, const void* memory, uint32_t mip_level, uint32_t offset_x, uint32_t offset_y)
{
	auto texture = (TextureD3D12*)handle;
	texture->Write(width, height, format, memory, mip_level, offset_x, offset_y);
}
//=============================================================================
void RHISystem::GenerateMips(TextureHandle* handle)
{
	auto texture = (TextureD3D12*)handle;
	texture->GenerateMips();
}
//=============================================================================
void RHISystem::DestroyTexture(TextureHandle* handle)
{
	auto texture = (TextureD3D12*)handle;
	delete texture;
}
//=============================================================================
RenderTargetHandle* RHISystem::CreateRenderTarget(uint32_t width, uint32_t height, TextureHandle* texture_handle)
{
	auto texture = (TextureD3D12*)texture_handle;
	auto render_target = new RenderTargetD3D12(width, height, texture);
	return (RenderTargetHandle*)render_target;
}
//=============================================================================
void RHISystem::DestroyRenderTarget(RenderTargetHandle* handle)
{
	auto render_target = (RenderTargetD3D12*)handle;
	delete render_target;
}
//=============================================================================
ShaderHandle* RHISystem::CreateShader(const std::string& vertex_code, const std::string& fragment_code, const std::vector<std::string>& defines)
{
	auto shader = new ShaderD3D12(vertex_code, fragment_code, defines);
	return (ShaderHandle*)shader;
}
//=============================================================================
void RHISystem::DestroyShader(ShaderHandle* handle)
{
	auto shader = (ShaderD3D12*)handle;
	delete shader;
}
//=============================================================================
VertexBufferHandle* RHISystem::CreateVertexBuffer(size_t size, size_t stride)
{
	auto buffer = new VertexBufferD3D12(size, stride);
	return (VertexBufferHandle*)buffer;
}
//=============================================================================
void RHISystem::DestroyVertexBuffer(VertexBufferHandle* handle)
{
	auto buffer = (VertexBufferD3D12*)handle;
	delete buffer;
}
//=============================================================================
void RHISystem::WriteVertexBufferMemory(VertexBufferHandle* handle, const void* memory, size_t size, size_t stride)
{
	auto buffer = (VertexBufferD3D12*)handle;
	buffer->Write(memory, size);
	buffer->SetStride(stride);

	for (auto vertex_buffer : gContext.vertexBuffers)
	{
		if (vertex_buffer != buffer)
			continue;

		gContext.vertexBuffersDirty = true;
		break;
	}
}
//=============================================================================
IndexBufferHandle* RHISystem::CreateIndexBuffer(size_t size, size_t stride)
{
	auto buffer = new IndexBufferD3D12(size, stride);
	return (IndexBufferHandle*)buffer;
}
//=============================================================================
void RHISystem::DestroyIndexBuffer(IndexBufferHandle* handle)
{
	auto buffer = (IndexBufferD3D12*)handle;
	delete buffer;
}
//=============================================================================
void RHISystem::WriteIndexBufferMemory(IndexBufferHandle* handle, const void* memory, size_t size, size_t stride)
{
	auto buffer = (IndexBufferD3D12*)handle;
	buffer->Write(memory, size);
	buffer->SetStride(stride);

	if (gContext.indexBuffer == buffer)
		gContext.indexBufferDirty = true;
}
//=============================================================================
UniformBufferHandle* RHISystem::CreateUniformBuffer(size_t size)
{
	auto buffer = new UniformBufferD3D12(size);
	return (UniformBufferHandle*)buffer;
}
//=============================================================================
void RHISystem::DestroyUniformBuffer(UniformBufferHandle* handle)
{
	auto buffer = (UniformBufferD3D12*)handle;
	delete buffer;
}
//=============================================================================
void RHISystem::WriteUniformBufferMemory(UniformBufferHandle* handle, const void* memory, size_t size)
{
	auto buffer = (UniformBufferD3D12*)handle;
	buffer->Write(memory, size);
}
//=============================================================================
#endif // RENDER_D3D12