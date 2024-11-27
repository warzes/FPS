#pragma once

#if RENDER_VULKAN

#include "RenderCoreVK.h"

class BufferVK : public ObjectVK
{
public:
	BufferVK(size_t size, vk::BufferUsageFlags usage);
	~BufferVK();

	void Write(const void* memory, size_t size);

	const auto& GetBuffer() const { return mBuffer; }

private:
	vk::raii::Buffer mBuffer = nullptr;
	vk::raii::DeviceMemory mDeviceMemory = nullptr;
};

class VertexBufferVK : public BufferVK
{
public:
	VertexBufferVK(size_t size, size_t stride) : BufferVK(size, vk::BufferUsageFlagBits::eVertexBuffer), mStride(stride) {}

	auto GetStride() const { return mStride; }
	void SetStride(size_t value) { mStride = value; }

private:
	size_t mStride = 0;
};

class IndexBufferVK : public BufferVK
{
public:
	IndexBufferVK(size_t size, size_t stride) : BufferVK(size, vk::BufferUsageFlagBits::eIndexBuffer), mStride(stride) {}

	auto GetStride() const { return mStride; }
	void SetStride(size_t value) { mStride = value; }

private:
	size_t mStride = 0;
};

class UniformBufferVK : public BufferVK
{
public:
	UniformBufferVK(size_t size) : BufferVK(size, vk::BufferUsageFlagBits::eUniformBuffer)
	{
	}
};

class StorageBufferVK : public BufferVK
{
public:
	StorageBufferVK(size_t size) : BufferVK(size, vk::BufferUsageFlagBits::eStorageBuffer)
	{
	}
};

#endif // RENDER_VULKAN