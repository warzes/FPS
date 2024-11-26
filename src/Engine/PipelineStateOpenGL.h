#pragma once

#if RENDER_OPENGL

#include "RenderCoreOpenGL.h"

struct SamplerStateGL
{
	Sampler sampler = Sampler::Linear;
	TextureAddress textureAddress = TextureAddress::Clamp;

	bool operator==(const SamplerStateGL&) const = default;
};

SE_MAKE_HASHABLE(SamplerStateGL,
	t.sampler,
	t.textureAddress
);

#endif // RENDER_OPENGL