#include "stdafx.h"
#if RENDER_VULKAN
#include "TextureVK.h"
#include "ContextVK.h"
//=============================================================================
TextureVK::TextureVK(uint32_t width, uint32_t height, vk::Format format, uint32_t mip_count) :
	mWidth(width),
	mHeight(height),
	mFormat(format),
	mMipCount(mip_count)
{
	auto usage =
		vk::ImageUsageFlagBits::eSampled |
		vk::ImageUsageFlagBits::eTransferDst |
		vk::ImageUsageFlagBits::eTransferSrc |
		vk::ImageUsageFlagBits::eColorAttachment |
		vk::ImageUsageFlagBits::eStorage;

	std::tie(mImage, mDeviceMemory, mImageView) = CreateImage(width, height, format, usage,
		vk::ImageAspectFlagBits::eColor, mip_count);

	mImagePtr = *mImage.value();
}
//=============================================================================
TextureVK::TextureVK(uint32_t width, uint32_t height, vk::Format format, vk::Image image) :
	mWidth(width),
	mHeight(height),
	mFormat(format),
	mImagePtr(image)
{
	mImageView = CreateImageView(image, format, vk::ImageAspectFlagBits::eColor);
}
//=============================================================================
TextureVK::~TextureVK()
{
	if (mImage.has_value())
		DestroyStaging(std::move(mImage.value()));

	if (mDeviceMemory.has_value())
		DestroyStaging(std::move(mDeviceMemory.value()));
}
//=============================================================================
void TextureVK::Write(uint32_t width, uint32_t height, PixelFormat format, const void* memory,
	uint32_t mip_level, uint32_t offset_x, uint32_t offset_y)
{
	EnsureRenderPassDeactivated();

	auto channels = GetFormatChannelsCount(format);
	auto channel_size = GetFormatChannelSize(format);
	auto size = width * height * channels * channel_size;

	auto [upload_buffer, upload_buffer_memory] = CreateBuffer(size, vk::BufferUsageFlagBits::eTransferSrc);

	WriteToBuffer(upload_buffer_memory, memory, size);

	EnsureState(gContext.getCurrentFrame().command_buffer, vk::ImageLayout::eTransferDstOptimal);

	auto image_subresource_layers = vk::ImageSubresourceLayers()
		.setAspectMask(vk::ImageAspectFlagBits::eColor)
		.setMipLevel(mip_level)
		.setLayerCount(1);

	auto region = vk::BufferImageCopy()
		.setImageSubresource(image_subresource_layers)
		.setImageExtent({ width, height, 1 });

	gContext.getCurrentFrame().command_buffer.copyBufferToImage(*upload_buffer, mImagePtr,
		vk::ImageLayout::eTransferDstOptimal, { region });

	DestroyStaging(std::move(upload_buffer));
	DestroyStaging(std::move(upload_buffer_memory));
}
//=============================================================================
void TextureVK::GenerateMips()
{
	EnsureState(gContext.getCurrentFrame().command_buffer, vk::ImageLayout::eTransferSrcOptimal);

	for (uint32_t i = 1; i < mMipCount; i++)
	{
		SetImageMemoryBarrier(gContext.getCurrentFrame().command_buffer, mImagePtr, vk::ImageAspectFlagBits::eColor,
			vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, i, 1);

		auto src_subresource = vk::ImageSubresourceLayers()
			.setAspectMask(vk::ImageAspectFlagBits::eColor)
			.setMipLevel(i - 1)
			.setLayerCount(1);

		auto dst_subresource = vk::ImageSubresourceLayers()
			.setAspectMask(vk::ImageAspectFlagBits::eColor)
			.setMipLevel(i)
			.setLayerCount(1);

		auto mip_region = vk::ImageBlit()
			.setSrcSubresource(src_subresource)
			.setDstSubresource(dst_subresource)
			.setSrcOffsets({ vk::Offset3D{ 0, 0, 0 }, vk::Offset3D{ int32_t(mWidth >> (i - 1)), int32_t(mHeight >> (i - 1)), 1 } })
			.setDstOffsets({ vk::Offset3D{ 0, 0, 0 }, vk::Offset3D{ int32_t(mWidth >> i), int32_t(mHeight >> i), 1 } });

		gContext.getCurrentFrame().command_buffer.blitImage(mImagePtr, vk::ImageLayout::eTransferSrcOptimal,
			mImagePtr, vk::ImageLayout::eTransferDstOptimal, { mip_region }, vk::Filter::eLinear);

		SetImageMemoryBarrier(gContext.getCurrentFrame().command_buffer, mImagePtr, vk::ImageAspectFlagBits::eColor,
			vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eTransferSrcOptimal, i, 1);
	}
}
//=============================================================================
void TextureVK::EnsureState(const vk::raii::CommandBuffer& cmdbuf, vk::ImageLayout state)
{
	if (mCurrentState == state)
		return;

	SetImageMemoryBarrier(cmdbuf, mImagePtr, vk::ImageAspectFlagBits::eColor, mCurrentState, state);
	mCurrentState = state;
}
//=============================================================================
#endif // RENDER_VULKAN