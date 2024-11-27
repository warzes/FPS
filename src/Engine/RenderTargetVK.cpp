#include "stdafx.h"
#if RENDER_VULKAN
#include "RenderTargetVK.h"
#include "ContextVK.h"
//=============================================================================
RenderTargetVK::RenderTargetVK(uint32_t width, uint32_t height, TextureVK* _texture) : mTexture(_texture)
{
	std::tie(mDepthStencilImage, mDepthStencilMemory, mDepthStencilView) = CreateImage(width, height, mDepthStencilFormat,
		vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil);

	OneTimeSubmit([&](auto& cmdbuf) {
		SetImageMemoryBarrier(cmdbuf, *mDepthStencilImage, mDepthStencilFormat, vk::ImageLayout::eUndefined,
			vk::ImageLayout::eDepthStencilAttachmentOptimal);
		});
}
//=============================================================================
#endif // RENDER_VULKAN