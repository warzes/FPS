#pragma once

#include "RenderResources.h"

namespace RHISystem
{
	TextureHandle* CreateTexture(uint32_t width, uint32_t height, PixelFormat format, uint32_t mipCount);
	void WriteTexturePixels(TextureHandle* handle, uint32_t width, uint32_t height, PixelFormat format, const void* memory, uint32_t mipLevel, uint32_t offsetX, uint32_t offsetY);
	void GenerateMips(TextureHandle* handle);
	void DestroyTexture(TextureHandle* handle);

	RenderTargetHandle* CreateRenderTarget(uint32_t width, uint32_t height, TextureHandle* texture);
	void DestroyRenderTarget(RenderTargetHandle* handle);

	ShaderHandle* CreateShader(const std::string& vertex_code, const std::string& fragment_code, const std::vector<std::string>& defines);
	void DestroyShader(ShaderHandle* handle);

	VertexBufferHandle* CreateVertexBuffer(size_t size, size_t stride);
	void DestroyVertexBuffer(VertexBufferHandle* handle);
	void WriteVertexBufferMemory(VertexBufferHandle* handle, const void* memory, size_t size, size_t stride);

	IndexBufferHandle* CreateIndexBuffer(size_t size, size_t stride);
	void DestroyIndexBuffer(IndexBufferHandle* handle);
	void WriteIndexBufferMemory(IndexBufferHandle* handle, const void* memory, size_t size, size_t stride);

	UniformBufferHandle* CreateUniformBuffer(size_t size);
	void DestroyUniformBuffer(UniformBufferHandle* handle);
	void WriteUniformBufferMemory(UniformBufferHandle* handle, const void* memory, size_t size);

#if RENDER_VULKAN
	RaytracingShaderHandle* CreateRaytracingShader(const std::string& raygen_code,
		const std::vector<std::string>& miss_code, const std::string& closesthit_code,
		const std::vector<std::string>& defines);
	void DestroyRaytracingShader(RaytracingShaderHandle* handle);

	BottomLevelAccelerationStructureHandle* CreateBottomLevelAccelerationStructure(const void* vertex_memory,
		uint32_t vertex_count, uint32_t vertex_stride, const void* index_memory, uint32_t index_count,
		uint32_t index_stride, const glm::mat4& transform);
	void DestroyBottomLevelAccelerationStructure(BottomLevelAccelerationStructureHandle* handle);

	TopLevelAccelerationStructureHandle* CreateTopLevelAccelerationStructure(
		const std::vector<std::tuple<uint32_t, BottomLevelAccelerationStructureHandle*>>& bottom_level_acceleration_structures);
	void DestroyTopLevelAccelerationStructure(TopLevelAccelerationStructureHandle* handle);

	StorageBufferHandle* CreateStorageBuffer(size_t size);
	void DestroyStorageBuffer(StorageBufferHandle* handle);
	void WriteStorageBufferMemory(StorageBufferHandle* handle, const void* memory, size_t size);
#endif // RENDER_VULKAN
}