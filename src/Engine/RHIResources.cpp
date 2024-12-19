#include "stdafx.h"
#include "RHIResources.h"
#include "RHIBackend.h"
//=============================================================================
Texture2D::Texture2D(uint32_t width, uint32_t height, PixelFormat format, uint32_t mipCount)
	: m_width(width)
	, m_height(height)
	, m_format(format)
	, m_mipCount(mipCount)
{
	assert(m_width > 0);
	assert(m_height > 0);
	assert(m_mipCount > 0);
	m_textureHandle = RHIBackend::CreateTexture(m_width, m_height, m_format, m_mipCount);
}
//=============================================================================
Texture2D::Texture2D(uint32_t width, uint32_t height, PixelFormat format, const void* memory, bool generateMips)
	: Texture2D(width, height, format, generateMips ? ::GetMipCount(width, height) : 1)
{
	Write(width, height, format, memory);

	if (generateMips)
		GenerateMips();
}
//=============================================================================
Texture2D::Texture2D(Texture2D&& other) noexcept
{
	m_textureHandle = std::exchange(other.m_textureHandle, nullptr);
	m_width         = std::exchange(other.m_width, 0);
	m_height        = std::exchange(other.m_height, 0);
	m_format        = std::exchange(other.m_format, {});
	m_mipCount      = std::exchange(other.m_mipCount, 0);
}
//=============================================================================
Texture2D::~Texture2D()
{
	RHIBackend::DestroyTexture(m_textureHandle);
}
//=============================================================================
void Texture2D::Write(uint32_t width, uint32_t height, PixelFormat format, const void* memory, uint32_t mipLevel, uint32_t offsetX, uint32_t offsetY)
{
	assert(width > 0);
	assert(height > 0);
	assert(offsetX + width <= GetMipWidth(m_width, mipLevel));
	assert(offsetY + height <= GetMipHeight(m_height, mipLevel));
	assert(mipLevel < m_mipCount);
	assert(memory != nullptr);
	RHIBackend::WriteTexturePixels(m_textureHandle, width, height, format, memory, mipLevel, offsetX, offsetY);
}
//=============================================================================
void Texture2D::GenerateMips()
{
	RHIBackend::GenerateMips(m_textureHandle);
}
//=============================================================================
Texture2D& Texture2D::operator=(Texture2D&& other) noexcept
{
	if (this == &other) return *this;

	if (m_textureHandle)
		RHIBackend::DestroyTexture(m_textureHandle);

	m_textureHandle = std::exchange(other.m_textureHandle, nullptr);
	m_width         = std::exchange(other.m_width, 0);
	m_height        = std::exchange(other.m_height, 0);
	m_format        = std::exchange(other.m_format, {});
	m_mipCount      = std::exchange(other.m_mipCount, 0);

	return *this;
}
//=============================================================================
RenderTarget::RenderTarget(uint32_t width, uint32_t height, PixelFormat format) 
	: Texture2D(width, height, format, 1)
{
	m_renderTargetHandle = RHIBackend::CreateRenderTarget(width, height, *this);
}
//=============================================================================
RenderTarget::RenderTarget(RenderTarget&& other) noexcept 
	: Texture2D(std::move(other))
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
	Texture2D::operator=(std::move(other));

	if (this == &other) return *this;

	if (m_renderTargetHandle)
		RHIBackend::DestroyRenderTarget(m_renderTargetHandle);

	m_renderTargetHandle = std::exchange(other.m_renderTargetHandle, nullptr);

	return *this;
}
//=============================================================================
Shader::Shader(const std::string& vertexCode, const std::string& fragmentCode, const std::vector<std::string>& defines)
{
	m_shaderHandle = RHIBackend::CreateShader(vertexCode, fragmentCode, defines);
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
	if (this == &other) return *this;

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

	if (this == &other) return *this;

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

	if (this == &other) return *this;

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

	if (this == &other) return *this;

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
RaytracingShader::RaytracingShader(const std::string& raygenCode, const std::vector<std::string>& missCode, const std::string& closesthitCode, const std::vector<std::string>& defines)
{
	m_raytracingShaderHandle = RHIBackend::CreateRaytracingShader(raygenCode, missCode, closesthitCode, defines);
}
#endif // RENDER_VULKAN
//=============================================================================
#if RENDER_VULKAN
RaytracingShader::~RaytracingShader()
{
	RHIBackend::DestroyRaytracingShader(m_raytracingShaderHandle);
}
#endif // RENDER_VULKAN
//=============================================================================
#if RENDER_VULKAN
StorageBuffer::StorageBuffer(size_t size) : Buffer(size)
{
	m_storageBufferHandle = RHIBackend::CreateStorageBuffer(size);
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
	m_storageBufferHandle = std::exchange(other.m_storageBufferHandle, nullptr);
}
#endif // RENDER_VULKAN
//=============================================================================
#if RENDER_VULKAN
StorageBuffer::~StorageBuffer()
{
	RHIBackend::DestroyStorageBuffer(m_storageBufferHandle);
}
#endif // RENDER_VULKAN
//=============================================================================
#if RENDER_VULKAN
StorageBuffer& StorageBuffer::operator=(StorageBuffer&& other) noexcept
{
	Buffer::operator=(std::move(other));

	if (this == &other) return *this;

	if (m_storageBufferHandle)
		RHIBackend::DestroyStorageBuffer(m_storageBufferHandle);

	m_storageBufferHandle = std::exchange(other.m_storageBufferHandle, nullptr);
	return *this;
}
#endif // RENDER_VULKAN
//=============================================================================
#if RENDER_VULKAN
void StorageBuffer::Write(const void* memory, size_t size)
{
	RHIBackend::WriteStorageBufferMemory(m_storageBufferHandle, memory, size);
}
#endif // RENDER_VULKAN
//=============================================================================
#if RENDER_VULKAN
BottomLevelAccelerationStructure::BottomLevelAccelerationStructure(const void* vertexMemory, uint32_t vertexCount, uint32_t vertexOffset, uint32_t vertexStride, const void* indexMemory, uint32_t indexCount, uint32_t indexOffset, uint32_t indexStride, const glm::mat4& transform)
{
	auto vertexMemoryWithOffset = (void*)((size_t)vertexMemory + vertexOffset);
	auto indexMemoryWithOffset = (void*)((size_t)indexMemory + indexOffset);

	m_bottomLevelAccelerationStructureHandle = RHIBackend::CreateBottomLevelAccelerationStructure(vertexMemoryWithOffset, vertexCount, vertexStride, indexMemoryWithOffset, indexCount, indexStride, transform);
}
#endif // RENDER_VULKAN
//=============================================================================
#if RENDER_VULKAN
BottomLevelAccelerationStructure::~BottomLevelAccelerationStructure()
{
	if (m_bottomLevelAccelerationStructureHandle)
		RHIBackend::DestroyBottomLevelAccelerationStructure(m_bottomLevelAccelerationStructureHandle);
}
#endif // RENDER_VULKAN
//=============================================================================
#if RENDER_VULKAN
TopLevelAccelerationStructure::TopLevelAccelerationStructure(const std::vector<std::tuple<uint32_t, BottomLevelAccelerationStructureHandle*>>& bottomLevelAccelerationStructures)
{
	m_topLevelAccelerationStructureHandle = RHIBackend::CreateTopLevelAccelerationStructure(bottomLevelAccelerationStructures);
}
#endif // RENDER_VULKAN
//=============================================================================
#if RENDER_VULKAN
TopLevelAccelerationStructure::TopLevelAccelerationStructure(const std::vector<BottomLevelAccelerationStructureHandle*>& bottomLevelAccelerationStructures) : TopLevelAccelerationStructure(createIndexedBlases(bottomLevelAccelerationStructures))
{
}
#endif // RENDER_VULKAN
//=============================================================================
#if RENDER_VULKAN
TopLevelAccelerationStructure::~TopLevelAccelerationStructure()
{
	if (m_topLevelAccelerationStructureHandle)
		RHIBackend::DestroyTopLevelAccelerationStructure(m_topLevelAccelerationStructureHandle);
}
#endif // RENDER_VULKAN
//=============================================================================
#if RENDER_VULKAN
std::vector<std::tuple<uint32_t, BottomLevelAccelerationStructureHandle*>>
TopLevelAccelerationStructure::createIndexedBlases(const std::vector<BottomLevelAccelerationStructureHandle*>& blases)
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