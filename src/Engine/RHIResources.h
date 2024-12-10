#pragma once

#include "RHICore.h"

class Texture : Noncopyable
{
public:
	Texture(uint32_t width, uint32_t height, PixelFormat format, uint32_t mipCount);
	Texture(uint32_t width, uint32_t height, PixelFormat format, const void* memory, bool generateMips = false);
	Texture(Texture&& other) noexcept;
	virtual ~Texture();

	void Write(uint32_t width, uint32_t height, PixelFormat format, const void* memory, uint32_t mipLevel = 0, uint32_t offsetX = 0, uint32_t offsetY = 0);
	void GenerateMips();

	Texture& operator=(Texture&& other) noexcept;

	operator TextureHandle*() { return m_textureHandle; }

	auto GetWidth() const { return m_width; }
	auto GetHeight() const { return m_height; }
	auto GetFormat() const { return m_format; }
	auto GetMipCount() const { return m_mipCount; }

private:
	TextureHandle* m_textureHandle{ nullptr };
	uint32_t       m_width{ 0 };
	uint32_t       m_height{ 0 };
	PixelFormat    m_format{};
	uint32_t       m_mipCount{ 0 };
};

class RenderTarget : public Texture
{
public:
	RenderTarget(uint32_t width, uint32_t height, PixelFormat format = PixelFormat::RGBA32Float);
	RenderTarget(RenderTarget&& other) noexcept;
	~RenderTarget();

	RenderTarget& operator=(RenderTarget&& other) noexcept;

	operator RenderTargetHandle*() { return m_renderTargetHandle; }
private:
	RenderTargetHandle* m_renderTargetHandle{ nullptr };
};

class Shader final : Noncopyable
{
public:
	Shader(const std::string& vertexCode, const std::string& fragmentCode, const std::vector<std::string>& defines = {});
	Shader(Shader&& other) noexcept;
	~Shader();

	operator ShaderHandle*() { return m_shaderHandle; }

	Shader& operator=(Shader&& other) noexcept;

private:
	ShaderHandle* m_shaderHandle{ nullptr };
};

class Buffer : Noncopyable
{
public:
	Buffer(size_t size);
	Buffer(Buffer&& other) noexcept;

	Buffer& operator=(Buffer&& other) noexcept;

	auto GetSize() const { return m_size; }

private:
	size_t m_size{ 0 };
};

class VertexBuffer final : public Buffer
{
public:
	VertexBuffer(size_t size, size_t stride);
	VertexBuffer(const void* memory, size_t size, size_t stride);
	VertexBuffer(VertexBuffer&& other) noexcept;
	~VertexBuffer();

	VertexBuffer& operator=(VertexBuffer&& other) noexcept;

	template<class T>
	explicit VertexBuffer(const T* memory, size_t count) : VertexBuffer(memory, count * sizeof(T), sizeof(T)) {}

	template<class T>
	explicit VertexBuffer(const std::vector<T>& values) : VertexBuffer(values.data(), values.size()) {}

	void Write(const void* memory, size_t size, size_t stride);

	template<class T>
	void Write(const T* memory, size_t count)
	{
		Write(memory, count * sizeof(T), sizeof(T));
	}

	template<class T>
	void Write(const std::vector<T>& values)
	{
		Write(values.data(), values.size());
	}

	operator VertexBufferHandle*() { return m_vertexBufferHandle; }

private:
	VertexBufferHandle* m_vertexBufferHandle{ nullptr };
};

class IndexBuffer final : public Buffer
{
public:
	IndexBuffer(size_t size, size_t stride);
	IndexBuffer(const void* memory, size_t size, size_t stride);
	IndexBuffer(IndexBuffer&& other) noexcept;
	~IndexBuffer();

	IndexBuffer& operator=(IndexBuffer&& other) noexcept;

	template<class T>
	explicit IndexBuffer(const T* memory, size_t count) : IndexBuffer(memory, count * sizeof(T), sizeof(T)) {}

	template<class T>
	explicit IndexBuffer(const std::vector<T>& values) : IndexBuffer(values.data(), values.size()) {}

	void Write(const void* memory, size_t size, size_t stride);

	template<class T>
	void Write(const T* memory, size_t count)
	{
		Write(memory, count * sizeof(T), sizeof(T));
	}

	template<class T>
	void Write(const std::vector<T>& values)
	{
		Write(values.data(), values.size());
	}

	operator IndexBufferHandle*() { return m_indexBufferHandle; }

private:
	IndexBufferHandle* m_indexBufferHandle{ nullptr };
};

class UniformBuffer final : public Buffer
{
public:
	UniformBuffer(size_t size);
	UniformBuffer(const void* memory, size_t size);
	UniformBuffer(UniformBuffer&& other) noexcept;
	~UniformBuffer();

	UniformBuffer& operator=(UniformBuffer&& other) noexcept;

	template <class T>
	explicit UniformBuffer(T value) : UniformBuffer(&value, sizeof(T)) {}

	void Write(const void* memory, size_t size);

	template <class T>
	void Write(const T& value) { Write(&const_cast<T&>(value), sizeof(T)); }

	operator UniformBufferHandle*() { return m_uniformBufferHandle; }

private:
	UniformBufferHandle* m_uniformBufferHandle{ nullptr };
};

#if RENDER_VULKAN

class RaytracingShader final : Noncopyable
{
public:
	RaytracingShader(const std::string& raygenCode, const std::vector<std::string>& missCode, const std::string& closesthitCode, const std::vector<std::string>& defines = {});
	~RaytracingShader();

	operator RaytracingShaderHandle*() { return m_raytracingShaderHandle; }

private:
	RaytracingShaderHandle* m_raytracingShaderHandle{ nullptr };
};

class StorageBuffer final : public Buffer
{
public:
	StorageBuffer(size_t size);
	StorageBuffer(const void* memory, size_t size);
	StorageBuffer(StorageBuffer&& other) noexcept;
	~StorageBuffer();

	StorageBuffer& operator=(StorageBuffer&& other) noexcept;

	template <class T>
	explicit StorageBuffer(T value) : StorageBuffer(&value, sizeof(T)) {}

	void Write(const void* memory, size_t size);

	template <class T>
	void Write(const T& value) { Write(&const_cast<T&>(value), sizeof(T)); }

	operator StorageBufferHandle*() { return m_storageBufferHandle; }

private:
	StorageBufferHandle* m_storageBufferHandle{ nullptr };
};

class BottomLevelAccelerationStructure final : Noncopyable
{
public:
	BottomLevelAccelerationStructure(
		const void* vertexMemory, uint32_t vertexCount, uint32_t vertexOffset,
		uint32_t vertexStride, const void* indexMemory, uint32_t indexCount, uint32_t indexOffset,
		uint32_t indexStride, const glm::mat4& transform);
	~BottomLevelAccelerationStructure();

	template<class Vertex, class Index>
	explicit BottomLevelAccelerationStructure(const Vertex* vertexMemory, uint32_t vertexCount, uint32_t vertexOffset, const Index* indexMemory, uint32_t indexCount, uint32_t indexOffset, const glm::mat4& transform) 
		: BottomLevelAccelerationStructure(vertexMemory, vertexCount, vertexOffset, sizeof(Vertex), indexMemory, indexCount, indexOffset, sizeof(Index), transform)
	{
	}

	template<class Vertex, class Index>
	explicit BottomLevelAccelerationStructure(const std::vector<Vertex>& vertices, uint32_t vertexOffset,
		const std::vector<Index>& indices, uint32_t indexOffset, const glm::mat4& transform)
		: BottomLevelAccelerationStructure(vertices.data(), (uint32_t)vertices.size(), vertexOffset,
			indices.data(), (uint32_t)indices.size(), indexOffset, transform)
	{
	}

	operator BottomLevelAccelerationStructureHandle*() { return m_bottomLevelAccelerationStructureHandle; }

private:
	BottomLevelAccelerationStructureHandle* m_bottomLevelAccelerationStructureHandle{ nullptr };
};

class TopLevelAccelerationStructure final : public Noncopyable
{
public:
	TopLevelAccelerationStructure(const std::vector<std::tuple<uint32_t, BottomLevelAccelerationStructureHandle*>>& bottomLevelAccelerationStructures);
	TopLevelAccelerationStructure(const std::vector<BottomLevelAccelerationStructureHandle*>& bottomLevelAccelerationStructures);
	~TopLevelAccelerationStructure();

	operator TopLevelAccelerationStructureHandle*() { return m_topLevelAccelerationStructureHandle; }

private:
	std::vector<std::tuple<uint32_t, BottomLevelAccelerationStructureHandle*>> createIndexedBlases(const std::vector<BottomLevelAccelerationStructureHandle*>& blases);

	TopLevelAccelerationStructureHandle* m_topLevelAccelerationStructureHandle{ nullptr };
};

using BLAS = BottomLevelAccelerationStructure;
using TLAS = TopLevelAccelerationStructure;
#endif // RENDER_VULKAN

struct TransientRenderTargetDesc final
{
	TransientRenderTargetDesc(uint32_t _width, uint32_t _height, PixelFormat _format) : width(_width), height(_height), format(_format) {}

	uint32_t    width;
	uint32_t    height;
	PixelFormat format;

	bool operator==(const TransientRenderTargetDesc&) const = default;
};

SE_MAKE_HASHABLE(TransientRenderTargetDesc,
	t.width,
	t.height,
	t.format
);

class TransientRenderTarget final : public RenderTarget
{
public:
	enum class State
	{
		Active,
		Inactive,
		Destroy
	};

public:
	using RenderTarget::RenderTarget;

public:
	auto GetState() const { return m_state; }
	void SetState(State value) { m_state = value; }

private:
	State m_state = State::Active;
};