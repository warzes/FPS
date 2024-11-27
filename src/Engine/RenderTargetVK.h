#pragma once

#if RENDER_VULKAN

#include "TextureVK.h"

class RenderTargetVK : public ObjectVK
{
public:
	RenderTargetVK(uint32_t width, uint32_t height, TextureVK* _texture);

	auto GetTexture() const { return mTexture; }
	auto GetDepthStencilFormat() const { return mDepthStencilFormat; }
	const auto& GetDepthStencilImage() const { return mDepthStencilImage; }
	const auto& GetDepthStencilView() const { return mDepthStencilView; }

private:
	TextureVK* mTexture;
	vk::Format mDepthStencilFormat = DefaultDepthStencilFormat;
	vk::raii::Image mDepthStencilImage = nullptr;
	vk::raii::ImageView mDepthStencilView = nullptr;
	vk::raii::DeviceMemory mDepthStencilMemory = nullptr;
};

#endif // RENDER_VULKAN