#pragma once

#include "RHICore.h"
#include "WindowStructs.h"

class RenderTarget;
class VertexBuffer;

namespace RHIBackend
{
	bool CreateAPI(const WindowData& data, const RenderSystemCreateInfo& createInfo);
	void DestroyAPI();
	void ResizeFrameBuffer(uint32_t width, uint32_t height);
	void Present();

	void Clear(const std::optional<glm::vec4>& color, const std::optional<float>& depth, const std::optional<uint8_t>& stencil);
	void Draw(uint32_t vertex_count, uint32_t vertex_offset, uint32_t instance_count);
	void DrawIndexed(uint32_t index_count, uint32_t index_offset, uint32_t instance_count);

	void ReadPixels(const glm::i32vec2& pos, const glm::i32vec2& size, TextureHandle* dst_texture);

	ShaderHandle* CreateShader(const std::string& vertexCode, const std::string& fragmentCode, const std::vector<std::string>& defines);
	void DestroyShader(ShaderHandle* handle);

	TextureHandle* CreateTexture(uint32_t width, uint32_t height, PixelFormat format, uint32_t mip_count);
	void WriteTexturePixels(TextureHandle* handle, uint32_t width, uint32_t height, PixelFormat format, const void* memory, uint32_t mip_level, uint32_t offset_x, uint32_t offset_y) ;
	void GenerateMips(TextureHandle* handle);
	void DestroyTexture(TextureHandle* handle);

	RenderTargetHandle* CreateRenderTarget(uint32_t width, uint32_t height, TextureHandle* texture);
	void DestroyRenderTarget(RenderTargetHandle* handle);

	VertexBufferHandle* CreateVertexBuffer(size_t size, size_t stride);
	void DestroyVertexBuffer(VertexBufferHandle* handle);
	void WriteVertexBufferMemory(VertexBufferHandle* handle, const void* memory, size_t size, size_t stride);

	IndexBufferHandle* CreateIndexBuffer(size_t size, size_t stride);
	void DestroyIndexBuffer(IndexBufferHandle* handle);
	void WriteIndexBufferMemory(IndexBufferHandle* handle, const void* memory, size_t size, size_t stride);

	UniformBufferHandle* CreateUniformBuffer(size_t size);
	void DestroyUniformBuffer(UniformBufferHandle* handle);
	void WriteUniformBufferMemory(UniformBufferHandle* handle, const void* memory, size_t size);

	void SetTopology(Topology topology);
	void SetViewport(std::optional<Viewport> viewport);
	void SetScissor(std::optional<Scissor> scissor);

	void SetBlendMode(const std::optional<BlendMode>& blend_mode);
	void SetDepthMode(const std::optional<DepthMode>& depth_mode);
	void SetStencilMode(const std::optional<StencilMode>& stencil_mode);
	void SetCullMode(CullMode cull_mode);
	void SetSampler(Sampler value);
	void SetTextureAddress(TextureAddress value);
	void SetFrontFace(FrontFace value);
	void SetDepthBias(const std::optional<DepthBias> depth_bias);

	void SetShader(ShaderHandle* handle);
	void SetInputLayout(const std::vector<InputLayout>& value);

	void SetTexture(uint32_t binding, TextureHandle* handle);
	void SetRenderTarget(const RenderTarget** render_target, size_t count);

	void SetVertexBuffer(const VertexBuffer** vertex_buffer, size_t count);
	void SetIndexBuffer(IndexBufferHandle* handle);
	void SetUniformBuffer(uint32_t binding, UniformBufferHandle* handle);

	// raytracing
#if RENDER_VULKAN
	RaytracingShaderHandle* CreateRaytracingShader(const std::string& raygen_code,
		const std::vector<std::string>& miss_code, const std::string& closesthit_code,
		const std::vector<std::string>& defines);
	void DestroyRaytracingShader(RaytracingShaderHandle* handle);

	BottomLevelAccelerationStructureHandle* CreateBottomLevelAccelerationStructure(const void* vertex_memory,
		uint32_t vertex_count, uint32_t vertex_stride, const void* index_memory, uint32_t index_count,
		uint32_t index_stride, const glm::mat4& transform);
	void DestroyBottomLevelAccelerationStructure(BottomLevelAccelerationStructureHandle* handle);

	TopLevelAccelerationStructureHandle* CreateTopLevelAccelerationStructure(const std::vector<std::tuple<uint32_t, BottomLevelAccelerationStructureHandle*>>& bottom_level_acceleration_structures);
	void DestroyTopLevelAccelerationStructure(TopLevelAccelerationStructureHandle* handle);

	StorageBufferHandle* CreateStorageBuffer(size_t size);
	void DestroyStorageBuffer(StorageBufferHandle* handle);
	void WriteStorageBufferMemory(StorageBufferHandle* handle, const void* memory, size_t size);

	void SetStorageBuffer(uint32_t binding, StorageBufferHandle* handle);
	void SetRaytracingShader(RaytracingShaderHandle* handle);
	void SetAccelerationStructure(uint32_t binding, TopLevelAccelerationStructureHandle* handle);

	void DispatchRays(uint32_t width, uint32_t height, uint32_t depth);
#endif // RENDER_VULKAN

} // namespace RHIBackend