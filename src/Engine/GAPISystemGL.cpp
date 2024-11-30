#include "stdafx.h"
#if RENDER_OPENGL
#include "GAPISystem.h"
#include "ContextOpenGL.h"
//=============================================================================
TextureHandle* GAPISystem::CreateTexture(uint32_t width, uint32_t height, PixelFormat format, uint32_t mip_count)
{
	auto texture = new TextureGL(width, height, format, mip_count);
	return (TextureHandle*)texture;
}
//=============================================================================
void GAPISystem::WriteTexturePixels(TextureHandle* handle, uint32_t width, uint32_t height, PixelFormat format, const void* memory, uint32_t mip_level, uint32_t offset_x, uint32_t offset_y)
{
	auto texture = (TextureGL*)handle;
	texture->Write(width, height, format, memory, mip_level, offset_x, offset_y);
}
//=============================================================================
void GAPISystem::GenerateMips(TextureHandle* handle)
{
	auto texture = (TextureGL*)handle;
	texture->GenerateMips();
}
//=============================================================================
void GAPISystem::DestroyTexture(TextureHandle* handle)
{
	auto texture = (TextureGL*)handle;
	delete texture;
}
//=============================================================================
RenderTargetHandle* GAPISystem::CreateRenderTarget(uint32_t width, uint32_t height, TextureHandle* texture_handle)
{
	auto texture = (TextureGL*)texture_handle;
	auto render_target = new RenderTargetGL(texture);
	return (RenderTargetHandle*)render_target;
}
//=============================================================================
void GAPISystem::DestroyRenderTarget(RenderTargetHandle* handle)
{
	auto render_target = (RenderTargetGL*)handle;
	delete render_target;
}
//=============================================================================
ShaderHandle* GAPISystem::CreateShader(const std::string& vertex_code, const std::string& fragment_code, const std::vector<std::string>& defines)
{
	auto shader = new ShaderGL(vertex_code, fragment_code, defines);
	return (ShaderHandle*)shader;
}
//=============================================================================
void GAPISystem::DestroyShader(ShaderHandle* handle)
{
	auto shader = (ShaderGL*)handle;
	delete shader;
}
//=============================================================================
VertexBufferHandle* GAPISystem::CreateVertexBuffer(size_t size, size_t stride)
{
	auto buffer = new VertexBufferGL(size, stride);
	return (VertexBufferHandle*)buffer;
}
//=============================================================================
void GAPISystem::DestroyVertexBuffer(VertexBufferHandle* handle)
{
	gContext.execute_after_present.add([handle] {
		auto buffer = (VertexBufferGL*)handle;
		delete buffer;
		});
}
//=============================================================================
void GAPISystem::WriteVertexBufferMemory(VertexBufferHandle* handle, const void* memory, size_t size, size_t stride)
{
	auto buffer = (VertexBufferGL*)handle;
	buffer->Write(memory, size);
	buffer->SetStride(stride);
}
//=============================================================================
IndexBufferHandle* GAPISystem::CreateIndexBuffer(size_t size, size_t stride)
{
	auto buffer = new IndexBufferGL(size, stride);
	return (IndexBufferHandle*)buffer;
}
//=============================================================================
void GAPISystem::DestroyIndexBuffer(IndexBufferHandle* handle)
{
	gContext.execute_after_present.add([handle] {
		auto buffer = (IndexBufferGL*)handle;

		if (gContext.index_buffer == buffer)
			gContext.index_buffer = nullptr;

		delete buffer;
		});
}
//=============================================================================
void GAPISystem::WriteIndexBufferMemory(IndexBufferHandle* handle, const void* memory, size_t size, size_t stride)
{
	auto buffer = (IndexBufferGL*)handle;
	buffer->Write(memory, size);
	buffer->SetStride(stride);
}
//=============================================================================
UniformBufferHandle* GAPISystem::CreateUniformBuffer(size_t size)
{
	auto buffer = new UniformBufferGL(size);
	return (UniformBufferHandle*)buffer;
}
//=============================================================================
void GAPISystem::DestroyUniformBuffer(UniformBufferHandle* handle)
{
	gContext.execute_after_present.add([handle] {
		auto buffer = (UniformBufferGL*)handle;
		delete buffer;
		});
}
//=============================================================================
void GAPISystem::WriteUniformBufferMemory(UniformBufferHandle* handle, const void* memory, size_t size)
{
	auto buffer = (UniformBufferGL*)handle;
	buffer->Write(memory, size);
}
//=============================================================================
#endif // RENDER_OPENGL