#include "stdafx.h"
#include "RenderResources.h"
#include "RenderSystem.h"
#include "GAPISystem.h"
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
	m_textureHandle = GAPISystem::CreateTexture(width, height, format, mip_count);
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
	GAPISystem::DestroyTexture(m_textureHandle);
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
	GAPISystem::WriteTexturePixels(m_textureHandle, width, height, format, memory, mip_level, offset_x, offset_y);
}
//=============================================================================
void Texture::GenerateMips()
{
	GAPISystem::GenerateMips(m_textureHandle);
}
//=============================================================================
Texture& Texture::operator=(Texture&& other) noexcept
{
	if (this == &other)
		return *this;

	if (m_textureHandle)
		GAPISystem::DestroyTexture(m_textureHandle);

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
	m_renderTargetHandle = GAPISystem::CreateRenderTarget(width, height, *this);
}
//=============================================================================
RenderTarget::RenderTarget(RenderTarget&& other) noexcept : Texture(std::move(other))
{
	m_renderTargetHandle = std::exchange(other.m_renderTargetHandle, nullptr);
}
//=============================================================================
RenderTarget::~RenderTarget()
{
	GAPISystem::DestroyRenderTarget(m_renderTargetHandle);
}
//=============================================================================
RenderTarget& RenderTarget::operator=(RenderTarget&& other) noexcept
{
	Texture::operator=(std::move(other));

	if (this == &other)
		return *this;

	if (m_renderTargetHandle)
		GAPISystem::DestroyRenderTarget(m_renderTargetHandle);

	m_renderTargetHandle = std::exchange(other.m_renderTargetHandle, nullptr);

	return *this;
}
//=============================================================================
Shader::Shader(const std::string& vertex_code, const std::string& fragment_code, const std::vector<std::string>& defines)
{
	m_shaderHandle = GAPISystem::CreateShader(vertex_code, fragment_code, defines);
}
//=============================================================================
Shader::Shader(Shader&& other) noexcept
{
	if (m_shaderHandle)
		GAPISystem::DestroyShader(m_shaderHandle);

	m_shaderHandle = std::exchange(other.m_shaderHandle, nullptr);
}
//=============================================================================
Shader::~Shader()
{
	GAPISystem::DestroyShader(m_shaderHandle);
}
//=============================================================================
Shader& Shader::operator=(Shader&& other) noexcept
{
	if (this == &other)
		return *this;

	if (m_shaderHandle)
		GAPISystem::DestroyShader(m_shaderHandle);

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
	m_vertexBufferHandle = GAPISystem::CreateVertexBuffer(size, stride);
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
		GAPISystem::DestroyVertexBuffer(m_vertexBufferHandle);

	m_vertexBufferHandle = std::exchange(other.m_vertexBufferHandle, nullptr);
}
//=============================================================================
VertexBuffer::~VertexBuffer()
{
	if (m_vertexBufferHandle)
		GAPISystem::DestroyVertexBuffer(m_vertexBufferHandle);
}
//=============================================================================
VertexBuffer& VertexBuffer::operator=(VertexBuffer&& other) noexcept
{
	Buffer::operator=(std::move(other));

	if (this == &other)
		return *this;

	if (m_vertexBufferHandle)
		GAPISystem::DestroyVertexBuffer(m_vertexBufferHandle);

	m_vertexBufferHandle = std::exchange(other.m_vertexBufferHandle, nullptr);
	return *this;
}
//=============================================================================
void VertexBuffer::Write(const void* memory, size_t size, size_t stride)
{
	GAPISystem::WriteVertexBufferMemory(m_vertexBufferHandle, memory, size, stride);
}
//=============================================================================
IndexBuffer::IndexBuffer(size_t size, size_t stride) : Buffer(size)
{
	m_indexBufferHandle = GAPISystem::CreateIndexBuffer(size, stride);
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
		GAPISystem::DestroyIndexBuffer(m_indexBufferHandle);
}
//=============================================================================
IndexBuffer& IndexBuffer::operator=(IndexBuffer&& other) noexcept
{
	Buffer::operator=(std::move(other));

	if (this == &other)
		return *this;

	if (m_indexBufferHandle)
		GAPISystem::DestroyIndexBuffer(m_indexBufferHandle);

	m_indexBufferHandle = std::exchange(other.m_indexBufferHandle, nullptr);
	return *this;
}
//=============================================================================
void IndexBuffer::Write(const void* memory, size_t size, size_t stride)
{
	GAPISystem::WriteIndexBufferMemory(m_indexBufferHandle, memory, size, stride);
}
//=============================================================================
UniformBuffer::UniformBuffer(size_t size) : Buffer(size)
{
	m_uniformBufferHandle = GAPISystem::CreateUniformBuffer(size);
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
		GAPISystem::DestroyUniformBuffer(m_uniformBufferHandle);
}
//=============================================================================
UniformBuffer& UniformBuffer::operator=(UniformBuffer&& other) noexcept
{
	Buffer::operator=(std::move(other));

	if (this == &other)
		return *this;

	if (m_uniformBufferHandle)
		GAPISystem::DestroyUniformBuffer(m_uniformBufferHandle);

	m_uniformBufferHandle = std::exchange(other.m_uniformBufferHandle, nullptr);
	return *this;
}
//=============================================================================
void UniformBuffer::Write(const void* memory, size_t size)
{
	GAPISystem::WriteUniformBufferMemory(m_uniformBufferHandle, memory, size);
}
//=============================================================================