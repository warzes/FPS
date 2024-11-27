#include "stdafx.h"
#if RENDER_VULKAN
#include "PipelineStateVK.h"
#include "ContextVK.h"
//=============================================================================
vk::raii::Sampler CreateSamplerState(const SamplerStateVK& sampler_state)
{
	static const std::unordered_map<Sampler, vk::Filter> FilterMap = {
		{ Sampler::Linear, vk::Filter::eLinear },
		{ Sampler::Nearest, vk::Filter::eNearest },
	};

	static const std::unordered_map<TextureAddress, vk::SamplerAddressMode> AddressModeMap = {
		{ TextureAddress::Clamp, vk::SamplerAddressMode::eClampToEdge },
		{ TextureAddress::Wrap, vk::SamplerAddressMode::eRepeat },
		{ TextureAddress::MirrorWrap, vk::SamplerAddressMode::eMirrorClampToEdge },
	};

	auto sampler_create_info = vk::SamplerCreateInfo()
		.setMagFilter(FilterMap.at(sampler_state.sampler))
		.setMinFilter(FilterMap.at(sampler_state.sampler))
		.setMipmapMode(vk::SamplerMipmapMode::eLinear)
		.setAddressModeU(AddressModeMap.at(sampler_state.texture_address))
		.setAddressModeV(AddressModeMap.at(sampler_state.texture_address))
		.setAddressModeW(AddressModeMap.at(sampler_state.texture_address))
		.setMinLod(-1000)
		.setMaxLod(1000)
		.setMaxAnisotropy(1.0f);

	return gContext.device.createSampler(sampler_create_info);
}
//=============================================================================
#endif // RENDER_VULKAN