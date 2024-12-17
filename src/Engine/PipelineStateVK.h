#pragma once

#if RENDER_VULKAN

#include "RenderCoreVK.h"

struct PipelineStateVK
{
	ShaderVK* shader = nullptr;
	std::vector<vk::Format> color_attachment_formats;
	std::optional<vk::Format> depth_stencil_format;
	std::vector<InputLayout> inputLayouts;

	bool operator==(const PipelineStateVK& other) const = default;
};

SE_MAKE_HASHABLE(PipelineStateVK,
	t.shader,
	t.color_attachment_formats,
	t.depth_stencil_format,
	t.inputLayouts
);

struct RaytracingPipelineStateVK
{
	RaytracingShaderVK* shader = nullptr;

	bool operator==(const RaytracingPipelineStateVK& other) const = default;
};

SE_MAKE_HASHABLE(RaytracingPipelineStateVK,
	t.shader
);

struct SamplerStateVK
{
	Sampler sampler = Sampler::Linear;
	TextureAddress texture_address = TextureAddress::Clamp;

	bool operator==(const SamplerStateVK& other) const = default;
};

SE_MAKE_HASHABLE(SamplerStateVK,
	t.sampler,
	t.texture_address
);

vk::raii::Sampler CreateSamplerState(const SamplerStateVK& sampler_state);

#endif // RENDER_VULKAN