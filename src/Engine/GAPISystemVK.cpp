#include "stdafx.h"
#if RENDER_VULKAN
#include "GAPISystem.h"
#include "ContextVK.h"
#include "RenderTargetVK.h"
#include "ShaderVK.h"
#include "BufferVK.h"
#include "AccelerationStructureVK.h"
//=============================================================================
TextureHandle* GAPISystem::CreateTexture(uint32_t width, uint32_t height, PixelFormat format, uint32_t mip_count)
{
	auto texture = new TextureVK(width, height, PixelFormatMap.at(format), mip_count);
	gContext.objects.insert(texture);
	return (TextureHandle*)texture;
}
//=============================================================================
void GAPISystem::WriteTexturePixels(TextureHandle* handle, uint32_t width, uint32_t height, PixelFormat format, const void* memory, uint32_t mip_level, uint32_t offset_x, uint32_t offset_y)
{
	auto texture = (TextureVK*)handle;
	texture->Write(width, height, format, memory, mip_level, offset_x, offset_y);
}
//=============================================================================
void GAPISystem::GenerateMips(TextureHandle* handle)
{
	auto texture = (TextureVK*)handle;
	texture->GenerateMips();
}
//=============================================================================
void GAPISystem::DestroyTexture(TextureHandle* handle)
{
	auto texture = (TextureVK*)handle;

	std::erase_if(gContext.textures, [&](const auto& item) {
		const auto& [binding, _texture] = item;
		return texture == _texture;
		});

	gContext.objects.erase(texture);
	delete texture;
}
//=============================================================================
RenderTargetHandle* GAPISystem::CreateRenderTarget(uint32_t width, uint32_t height, TextureHandle* texture_handle)
{
	auto texture = (TextureVK*)texture_handle;
	auto render_target = new RenderTargetVK(width, height, texture);
	gContext.objects.insert(render_target);
	return (RenderTargetHandle*)render_target;
}
//=============================================================================
void GAPISystem::DestroyRenderTarget(RenderTargetHandle* handle)
{
	auto render_target = (RenderTargetVK*)handle;
	gContext.objects.erase(render_target);
	delete render_target;
}
//=============================================================================
ShaderHandle* GAPISystem::CreateShader(const std::string& vertex_code, const std::string& fragment_code, const std::vector<std::string>& defines)
{
	auto shader = new ShaderVK(vertex_code, fragment_code, defines);
	gContext.objects.insert(shader);
	return (ShaderHandle*)shader;
}
//=============================================================================
void GAPISystem::DestroyShader(ShaderHandle* handle)
{
	auto shader = (ShaderVK*)handle;

	for (auto& [state, pipeline] : gContext.pipeline_states)
	{
		if (state.shader != shader)
			continue;

		DestroyStaging(std::move(pipeline));
	}

	std::erase_if(gContext.pipeline_states, [&](const auto& item) {
		const auto& [state, pipeline] = item;
		return state.shader == shader;
		});

	gContext.objects.erase(shader);
	delete shader;
}
//=============================================================================
RaytracingShaderHandle* GAPISystem::CreateRaytracingShader(const std::string& raygen_code, const std::vector<std::string>& miss_code, const std::string& closesthit_code, const std::vector<std::string>& defines)
{
	auto shader = new RaytracingShaderVK(raygen_code, miss_code, closesthit_code, defines);
	gContext.objects.insert(shader);
	return (RaytracingShaderHandle*)shader;
}
//=============================================================================
void GAPISystem::DestroyRaytracingShader(RaytracingShaderHandle* handle)
{
	auto shader = (RaytracingShaderVK*)handle;

	std::erase_if(gContext.raytracing_pipeline_states, [&](const auto& item) {
		const auto& [state, pipeline] = item;
		return state.shader == shader;
		});

	gContext.objects.erase(shader);
	delete shader;
}
//=============================================================================
VertexBufferHandle* GAPISystem::CreateVertexBuffer(size_t size, size_t stride)
{
	auto buffer = new VertexBufferVK(size, stride);
	gContext.objects.insert(buffer);
	return (VertexBufferHandle*)buffer;
}
//=============================================================================
void GAPISystem::DestroyVertexBuffer(VertexBufferHandle* handle)
{
	auto buffer = (VertexBufferVK*)handle;
	gContext.objects.erase(buffer);
	delete buffer;
}
//=============================================================================
void GAPISystem::WriteVertexBufferMemory(VertexBufferHandle* handle, const void* memory, size_t size, size_t stride)
{
	auto buffer = (VertexBufferVK*)handle;
	buffer->Write(memory, size);
	buffer->SetStride(stride);

	auto has_buffer = std::ranges::any_of(gContext.vertex_buffers, [&](auto vertex_buffer) {
		return vertex_buffer == buffer;
		});

	if (has_buffer)
		gContext.vertex_buffers_dirty = true;
}
//=============================================================================
IndexBufferHandle* GAPISystem::CreateIndexBuffer(size_t size, size_t stride)
{
	auto buffer = new IndexBufferVK(size, stride);
	gContext.objects.insert(buffer);
	return (IndexBufferHandle*)buffer;
}
//=============================================================================
void GAPISystem::DestroyIndexBuffer(IndexBufferHandle* handle)
{
	auto buffer = (IndexBufferVK*)handle;
	gContext.objects.erase(buffer);
	delete buffer;
}
//=============================================================================
void GAPISystem::WriteIndexBufferMemory(IndexBufferHandle* handle, const void* memory, size_t size, size_t stride)
{
	auto buffer = (IndexBufferVK*)handle;
	buffer->Write(memory, size);
	buffer->SetStride(stride);

	if (gContext.index_buffer == buffer)
		gContext.index_buffer_dirty = true;
}
//=============================================================================
UniformBufferHandle* GAPISystem::CreateUniformBuffer(size_t size)
{
	auto buffer = new UniformBufferVK(size);
	gContext.objects.insert(buffer);
	return (UniformBufferHandle*)buffer;
}
//=============================================================================
void GAPISystem::DestroyUniformBuffer(UniformBufferHandle* handle)
{
	auto buffer = (UniformBufferVK*)handle;

	std::erase_if(gContext.uniform_buffers, [&](const auto& item) {
		const auto& [binding, _buffer] = item;
		return buffer == _buffer;
		});

	gContext.objects.erase(buffer);
	delete buffer;
}
//=============================================================================
void GAPISystem::WriteUniformBufferMemory(UniformBufferHandle* handle, const void* memory, size_t size)
{
	auto buffer = (UniformBufferVK*)handle;
	buffer->Write(memory, size);
}
//=============================================================================
BottomLevelAccelerationStructureHandle* GAPISystem::CreateBottomLevelAccelerationStructure(const void* vertex_memory,
	uint32_t vertex_count, uint32_t vertex_stride, const void* index_memory, uint32_t index_count,
	uint32_t index_stride, const glm::mat4& transform)
{
	auto bottom_level_acceleration_structure = new BottomLevelAccelerationStructureVK(vertex_memory,
		vertex_count, vertex_stride, index_memory, index_count, index_stride, transform);
	gContext.objects.insert(bottom_level_acceleration_structure);
	return (BottomLevelAccelerationStructureHandle*)bottom_level_acceleration_structure;
}
//=============================================================================
void GAPISystem::DestroyBottomLevelAccelerationStructure(BottomLevelAccelerationStructureHandle* handle)
{
	auto bottom_level_acceleration_structure = (BottomLevelAccelerationStructureVK*)handle;
	gContext.objects.erase(bottom_level_acceleration_structure);
	delete bottom_level_acceleration_structure;
}
//=============================================================================
TopLevelAccelerationStructureHandle* GAPISystem::CreateTopLevelAccelerationStructure(
	const std::vector<std::tuple<uint32_t, BottomLevelAccelerationStructureHandle*>>& bottom_level_acceleration_structures)
{
	auto top_level_acceleration_structure = new TopLevelAccelerationStructureVK(bottom_level_acceleration_structures);
	gContext.objects.insert(top_level_acceleration_structure);
	return (TopLevelAccelerationStructureHandle*)top_level_acceleration_structure;
}
//=============================================================================
void GAPISystem::DestroyTopLevelAccelerationStructure(TopLevelAccelerationStructureHandle* handle)
{
	auto top_level_acceleration_structure = (TopLevelAccelerationStructureVK*)handle;

	std::erase_if(gContext.top_level_acceleration_structures, [&](const auto& item) {
		const auto& [binding, _acceleration_structure] = item;
		return top_level_acceleration_structure == _acceleration_structure;
		});

	gContext.objects.erase(top_level_acceleration_structure);
	delete top_level_acceleration_structure;
}
//=============================================================================
StorageBufferHandle* GAPISystem::CreateStorageBuffer(size_t size)
{
	auto buffer = new StorageBufferVK(size);
	gContext.objects.insert(buffer);
	return (StorageBufferHandle*)buffer;
}
//=============================================================================
void GAPISystem::DestroyStorageBuffer(StorageBufferHandle* handle)
{
	auto buffer = (StorageBufferVK*)handle;

	std::erase_if(gContext.storage_buffers, [&](const auto& item) {
		const auto& [binding, _buffer] = item;
		return buffer == _buffer;
		});

	gContext.objects.erase(buffer);
	delete buffer;
}
//=============================================================================
void GAPISystem::WriteStorageBufferMemory(StorageBufferHandle* handle, const void* memory, size_t size)
{
	auto buffer = (StorageBufferVK*)handle;
	buffer->Write(memory, size);
}
//=============================================================================
#endif // RENDER_VULKAN