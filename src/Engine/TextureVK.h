#pragma once

#if RENDER_VULKAN

#include "RenderCoreVK.h"

class TextureVK : public ObjectVK
{
public:
	TextureVK(uint32_t width, uint32_t height, vk::Format format, uint32_t mip_count);
	TextureVK(uint32_t width, uint32_t height, vk::Format format, vk::Image image);
	~TextureVK();

	void Write(uint32_t width, uint32_t height, PixelFormat format, const void* memory, uint32_t mip_level, uint32_t offset_x, uint32_t offset_y);

	void GenerateMips();

	void EnsureState(const vk::raii::CommandBuffer& cmdbuf, vk::ImageLayout state);

	auto GetImage() const { return mImagePtr; }
	const auto& GetImageView() const { return mImageView; }
	auto GetFormat() const { return mFormat; }
	auto GetWidth() const { return mWidth; }
	auto GetHeight() const { return mHeight; }

private:
	std::optional<vk::raii::Image> mImage;
	std::optional<vk::raii::DeviceMemory> mDeviceMemory;
	vk::Image mImagePtr;
	vk::raii::ImageView mImageView = nullptr;
	uint32_t mWidth = 0;
	uint32_t mHeight = 0;
	uint32_t mMipCount = 0;
	vk::Format mFormat;
	vk::ImageLayout mCurrentState = vk::ImageLayout::eUndefined;
};

#endif // RENDER_VULKAN