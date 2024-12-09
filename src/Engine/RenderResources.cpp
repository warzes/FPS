#include "stdafx.h"
#include "RenderResources.h"
#include "RenderSystem.h"
#include "RHIBackend.h"
//=============================================================================
Texture::Texture(uint32_t width, uint32_t height, PixelFormat format, uint32_t mip_count)
	: m_width(width)
	, m_height(height)
	, m_format(format)
	, m_mipCount(mip_count)
{
	assert(width > 0);
	assert(height > 0);
	assert(mip_count > 0);
	m_textureHandle = RHIBackend::CreateTexture(width, height, format, mip_count);
}
//=============================================================================
Texture::Texture(uint32_t width, uint32_t height, PixelFormat format, const void* memory, bool generate_mips)
	: Texture(width, height, format, generate_mips ? ::GetMipCount(width, height) : 1)
{
	Write(width, height, format, memory);

	if (generate_mips)
		GenerateMips();
}
//=============================================================================
Texture::Texture(Texture&& other) noexcept
{
	m_textureHandle = std::exchange(other.m_textureHandle, nullptr);
	m_width = std::exchange(other.m_width, 0);
	m_height = std::exchange(other.m_height, 0);
	m_format = std::exchange(other.m_format, {});
	m_mipCount = std::exchange(other.m_mipCount, 0);
}
//=============================================================================
Texture::~Texture()
{
	RHIBackend::DestroyTexture(m_textureHandle);
}
//=============================================================================
void Texture::Write(uint32_t width, uint32_t height, PixelFormat format, const void* memory,
	uint32_t mip_level, uint32_t offset_x, uint32_t offset_y)
{
	assert(width > 0);
	assert(height > 0);
	assert(offset_x + width <= GetMipWidth(m_width, mip_level));
	assert(offset_y + height <= GetMipHeight(m_height, mip_level));
	assert(mip_level < m_mipCount);
	assert(memory != nullptr);
	RHIBackend::WriteTexturePixels(m_textureHandle, width, height, format, memory, mip_level, offset_x, offset_y);
}
//=============================================================================
void Texture::GenerateMips()
{
	RHIBackend::GenerateMips(m_textureHandle);
}
//=============================================================================
Texture& Texture::operator=(Texture&& other) noexcept
{
	if (this == &other)
		return *this;

	if (m_textureHandle)
		RHIBackend::DestroyTexture(m_textureHandle);

	m_textureHandle = std::exchange(other.m_textureHandle, nullptr);
	m_width = std::exchange(other.m_width, 0);
	m_height = std::exchange(other.m_height, 0);
	m_format = std::exchange(other.m_format, {});
	m_mipCount = std::exchange(other.m_mipCount, 0);

	return *this;
}
//=============================================================================
RenderTarget::RenderTarget(uint32_t width, uint32_t height, PixelFormat format) : Texture(width, height, format, 1)
{
	m_renderTargetHandle = RHIBackend::CreateRenderTarget(width, height, *this);
}
//=============================================================================
RenderTarget::RenderTarget(RenderTarget&& other) noexcept : Texture(std::move(other))
{
	m_renderTargetHandle = std::exchange(other.m_renderTargetHandle, nullptr);
}
//=============================================================================
RenderTarget::~RenderTarget()
{
	RHIBackend::DestroyRenderTarget(m_renderTargetHandle);
}
//=============================================================================
RenderTarget& RenderTarget::operator=(RenderTarget&& other) noexcept
{
	Texture::operator=(std::move(other));

	if (this == &other)
		return *this;

	if (m_renderTargetHandle)
		RHIBackend::DestroyRenderTarget(m_renderTargetHandle);

	m_renderTargetHandle = std::exchange(other.m_renderTargetHandle, nullptr);

	return *this;
}
//=============================================================================
Shader::Shader(const std::string& vertex_code, const std::string& fragment_code, const std::vector<std::string>& defines)
{
	m_shaderHandle = RHIBackend::CreateShader(vertex_code, fragment_code, defines);
}
//=============================================================================
Shader::Shader(Shader&& other) noexcept
{
	RHIBackend::DestroyShader(m_shaderHandle);

	m_shaderHandle = std::exchange(other.m_shaderHandle, nullptr);
}
//=============================================================================
Shader::~Shader()
{
	RHIBackend::DestroyShader(m_shaderHandle);
}
//=============================================================================
Shader& Shader::operator=(Shader&& other) noexcept
{
	if (this == &other) return *this;

	RHIBackend::DestroyShader(m_shaderHandle);

	m_shaderHandle = std::exchange(other.m_shaderHandle, nullptr);
	return *this;
}
//=============================================================================
Buffer::Buffer(size_t size) : m_size(size)
{
	assert(size > 0);
}
//=============================================================================
Buffer::Buffer(Buffer&& other) noexcept
{
	m_size = std::exchange(other.m_size, 0);
}
//=============================================================================
Buffer& Buffer::operator=(Buffer&& other) noexcept
{
	if (this == &other)
		return *this;

	m_size = std::exchange(other.m_size, 0);
	return *this;
}
//=============================================================================
VertexBuffer::VertexBuffer(size_t size, size_t stride) : Buffer(size)
{
	m_vertexBufferHandle = RHIBackend::CreateVertexBuffer(size, stride);
}
//=============================================================================
VertexBuffer::VertexBuffer(const void* memory, size_t size, size_t stride) : VertexBuffer(size, stride)
{
	Write(memory, size, stride);
}
//=============================================================================
VertexBuffer::VertexBuffer(VertexBuffer&& other) noexcept : Buffer(std::move(other))
{
	if (m_vertexBufferHandle)
		RHIBackend::DestroyVertexBuffer(m_vertexBufferHandle);

	m_vertexBufferHandle = std::exchange(other.m_vertexBufferHandle, nullptr);
}
//=============================================================================
VertexBuffer::~VertexBuffer()
{
	if (m_vertexBufferHandle)
		RHIBackend::DestroyVertexBuffer(m_vertexBufferHandle);
}
//=============================================================================
VertexBuffer& VertexBuffer::operator=(VertexBuffer&& other) noexcept
{
	Buffer::operator=(std::move(other));

	if (this == &other)
		return *this;

	if (m_vertexBufferHandle)
		RHIBackend::DestroyVertexBuffer(m_vertexBufferHandle);

	m_vertexBufferHandle = std::exchange(other.m_vertexBufferHandle, nullptr);
	return *this;
}
//=============================================================================
void VertexBuffer::Write(const void* memory, size_t size, size_t stride)
{
	RHIBackend::WriteVertexBufferMemory(m_vertexBufferHandle, memory, size, stride);
}
//=============================================================================
IndexBuffer::IndexBuffer(size_t size, size_t stride) : Buffer(size)
{
	m_indexBufferHandle = RHIBackend::CreateIndexBuffer(size, stride);
}
//=============================================================================
IndexBuffer::IndexBuffer(const void* memory, size_t size, size_t stride) : IndexBuffer(size, stride)
{
	Write(memory, size, stride);
}
//=============================================================================
IndexBuffer::IndexBuffer(IndexBuffer&& other) noexcept : Buffer(std::move(other))
{
	m_indexBufferHandle = std::exchange(other.m_indexBufferHandle, nullptr);
}
//=============================================================================
IndexBuffer::~IndexBuffer()
{
	if (m_indexBufferHandle)
		RHIBackend::DestroyIndexBuffer(m_indexBufferHandle);
}
//=============================================================================
IndexBuffer& IndexBuffer::operator=(IndexBuffer&& other) noexcept
{
	Buffer::operator=(std::move(other));

	if (this == &other)
		return *this;

	if (m_indexBufferHandle)
		RHIBackend::DestroyIndexBuffer(m_indexBufferHandle);

	m_indexBufferHandle = std::exchange(other.m_indexBufferHandle, nullptr);
	return *this;
}
//=============================================================================
void IndexBuffer::Write(const void* memory, size_t size, size_t stride)
{
	RHIBackend::WriteIndexBufferMemory(m_indexBufferHandle, memory, size, stride);
}
//=============================================================================
UniformBuffer::UniformBuffer(size_t size) : Buffer(size)
{
	m_uniformBufferHandle = RHIBackend::CreateUniformBuffer(size);
}
//=============================================================================
UniformBuffer::UniformBuffer(const void* memory, size_t size) : UniformBuffer(size)
{
	Write(memory, size);
}
//=============================================================================
UniformBuffer::UniformBuffer(UniformBuffer&& other) noexcept : Buffer(std::move(other))
{
	m_uniformBufferHandle = std::exchange(other.m_uniformBufferHandle, nullptr);
}
//=============================================================================
UniformBuffer::~UniformBuffer()
{
	if (m_uniformBufferHandle)
		RHIBackend::DestroyUniformBuffer(m_uniformBufferHandle);
}
//=============================================================================
UniformBuffer& UniformBuffer::operator=(UniformBuffer&& other) noexcept
{
	Buffer::operator=(std::move(other));

	if (this == &other)
		return *this;

	if (m_uniformBufferHandle)
		RHIBackend::DestroyUniformBuffer(m_uniformBufferHandle);

	m_uniformBufferHandle = std::exchange(other.m_uniformBufferHandle, nullptr);
	return *this;
}
//=============================================================================
void UniformBuffer::Write(const void* memory, size_t size)
{
	RHIBackend::WriteUniformBufferMemory(m_uniformBufferHandle, memory, size);
}
//=============================================================================
#if RENDER_VULKAN
RaytracingShader::RaytracingShader(const std::string& raygen_code, const std::vector<std::string>& miss_code,
	const std::string& closesthit_code, const std::vector<std::string>& defines)
{
	mRaytracingShaderHandle = RHIBackend::CreateRaytracingShader(raygen_code, miss_code, closesthit_code, defines);
}
#endif // RENDER_VULKAN
//=============================================================================
#if RENDER_VULKAN
RaytracingShader::~RaytracingShader()
{
	RHIBackend::DestroyRaytracingShader(mRaytracingShaderHandle);
}
#endif // RENDER_VULKAN
//=============================================================================
#if RENDER_VULKAN
StorageBuffer::StorageBuffer(size_t size) : Buffer(size)
{
	mStorageBufferHandle = RHIBackend::CreateStorageBuffer(size);
}
#endif // RENDER_VULKAN
//=============================================================================
#if RENDER_VULKAN
StorageBuffer::StorageBuffer(const void* memory, size_t size) : StorageBuffer(size)
{
	Write(memory, size);
}
#endif // RENDER_VULKAN
//=============================================================================
#if RENDER_VULKAN

StorageBuffer::StorageBuffer(StorageBuffer&& other) noexcept : Buffer(std::move(other))
{
	mStorageBufferHandle = std::exchange(other.mStorageBufferHandle, nullptr);
}
#endif // RENDER_VULKAN
//=============================================================================
#if RENDER_VULKAN
StorageBuffer::~StorageBuffer()
{
	if (mStorageBufferHandle)
		RHIBackend::DestroyStorageBuffer(mStorageBufferHandle);
}
#endif // RENDER_VULKAN
//=============================================================================
#if RENDER_VULKAN
StorageBuffer& StorageBuffer::operator=(StorageBuffer&& other) noexcept
{
	Buffer::operator=(std::move(other));

	if (this == &other)
		return *this;

	if (mStorageBufferHandle)
		RHIBackend::DestroyStorageBuffer(mStorageBufferHandle);

	mStorageBufferHandle = std::exchange(other.mStorageBufferHandle, nullptr);
	return *this;
}
#endif // RENDER_VULKAN
//=============================================================================
#if RENDER_VULKAN
void StorageBuffer::Write(const void* memory, size_t size)
{
	RHIBackend::WriteStorageBufferMemory(mStorageBufferHandle, memory, size);
}
#endif // RENDER_VULKAN
//=============================================================================
#if RENDER_VULKAN
BottomLevelAccelerationStructure::BottomLevelAccelerationStructure(const void* vertex_memory, uint32_t vertex_count,
	uint32_t vertex_offset, uint32_t vertex_stride, const void* index_memory, uint32_t index_count,
	uint32_t index_offset, uint32_t index_stride, const glm::mat4& transform)
{
	auto vertex_memory_with_offset = (void*)((size_t)vertex_memory + vertex_offset);
	auto index_memory_with_offset = (void*)((size_t)index_memory + index_offset);

	mBottomLevelAccelerationStructureHandle = RHIBackend::CreateBottomLevelAccelerationStructure(vertex_memory_with_offset,
		vertex_count, vertex_stride, index_memory_with_offset, index_count, index_stride, transform);
}
#endif // RENDER_VULKAN
//=============================================================================
#if RENDER_VULKAN
BottomLevelAccelerationStructure::~BottomLevelAccelerationStructure()
{
	if (mBottomLevelAccelerationStructureHandle)
		RHIBackend::DestroyBottomLevelAccelerationStructure(mBottomLevelAccelerationStructureHandle);
}
#endif // RENDER_VULKAN
//=============================================================================
#if RENDER_VULKAN
TopLevelAccelerationStructure::TopLevelAccelerationStructure(const std::vector<std::tuple<uint32_t, BottomLevelAccelerationStructureHandle*>>& bottom_level_acceleration_structures)
{
	mTopLevelAccelerationStructureHandle = RHIBackend::CreateTopLevelAccelerationStructure(bottom_level_acceleration_structures);
}
#endif // RENDER_VULKAN
//=============================================================================
#if RENDER_VULKAN
TopLevelAccelerationStructure::TopLevelAccelerationStructure(const std::vector<BottomLevelAccelerationStructureHandle*>& bottom_level_acceleration_structures) : TopLevelAccelerationStructure(CreateIndexedBlases(bottom_level_acceleration_structures))
{
}
#endif // RENDER_VULKAN
//=============================================================================
#if RENDER_VULKAN
TopLevelAccelerationStructure::~TopLevelAccelerationStructure()
{
	if (mTopLevelAccelerationStructureHandle)
		RHIBackend::DestroyTopLevelAccelerationStructure(mTopLevelAccelerationStructureHandle);
}
#endif // RENDER_VULKAN
//=============================================================================
#if RENDER_VULKAN
std::vector<std::tuple<uint32_t, BottomLevelAccelerationStructureHandle*>>
TopLevelAccelerationStructure::CreateIndexedBlases(const std::vector<BottomLevelAccelerationStructureHandle*>& blases)
{
	std::vector<std::tuple<uint32_t, BottomLevelAccelerationStructureHandle*>> result;
	for (auto blas : blases)
	{
		result.push_back({ (uint32_t)result.size(), blas });
	}
	return result;
}
#endif // RENDER_VULKAN
//=============================================================================