#include "stdafx.h"
#if RENDER_VULKAN
#include "BufferVK.h"
#include "ContextVK.h"
//=============================================================================
BufferVK::BufferVK(size_t size, vk::BufferUsageFlags usage)
{
	usage |= vk::BufferUsageFlagBits::eTransferDst;
	std::tie(mBuffer, mDeviceMemory) = CreateBuffer(size, usage);
}
//=============================================================================
BufferVK::~BufferVK()
{
	DestroyStaging(std::move(mBuffer));
	DestroyStaging(std::move(mDeviceMemory));
}
//=============================================================================
void BufferVK::Write(const void* memory, size_t size)
{
	EnsureRenderPassDeactivated();
	EnsureMemoryState(gContext.getCurrentFrame().command_buffer, vk::PipelineStageFlagBits2::eTransfer);

	if (size < 65536)
	{
		gContext.getCurrentFrame().command_buffer.updateBuffer<uint8_t>(*mBuffer, 0, { (uint32_t)size, (uint8_t*)memory });
		return;
	}

	auto [staging_buffer, staging_buffer_memory] = CreateBuffer(size, vk::BufferUsageFlagBits::eTransferSrc);

	WriteToBuffer(staging_buffer_memory, memory, size);

	auto region = vk::BufferCopy()
		.setSize(size);

	gContext.getCurrentFrame().command_buffer.copyBuffer(*staging_buffer, *mBuffer, { region });

	DestroyStaging(std::move(staging_buffer));
	DestroyStaging(std::move(staging_buffer_memory));
}
//=============================================================================
#endif // RENDER_VULKAN