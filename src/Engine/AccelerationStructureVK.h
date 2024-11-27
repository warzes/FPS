#pragma once

#if RENDER_VULKAN

#include "RenderCoreVK.h"
#include "RenderResources.h"

class BottomLevelAccelerationStructureVK : public ObjectVK
{
public:
	BottomLevelAccelerationStructureVK(const void* vertex_memory, uint32_t vertex_count, uint32_t vertex_stride, const void* index_memory, uint32_t index_count, uint32_t index_stride, const glm::mat4& _transform);
	~BottomLevelAccelerationStructureVK();

	const auto& GetBlas() const { return mBlas; }

private:
	vk::raii::AccelerationStructureKHR mBlas = nullptr;
	vk::raii::Buffer mBlasBuffer = nullptr;
	vk::raii::DeviceMemory mBlasMemory = nullptr;
};

class TopLevelAccelerationStructureVK : public ObjectVK
{
public:
	TopLevelAccelerationStructureVK(const std::vector<std::tuple<uint32_t, BottomLevelAccelerationStructureHandle*>>& bottom_level_acceleration_structures);
	~TopLevelAccelerationStructureVK();

	const auto& GetTlas() const { return mTlas; }

private:
	vk::raii::AccelerationStructureKHR mTlas = nullptr;
	vk::raii::Buffer mTlasBuffer = nullptr;
	vk::raii::DeviceMemory mTlasMemory = nullptr;
};

#endif // RENDER_VULKAN