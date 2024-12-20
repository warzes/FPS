#pragma once

#if RENDER_OPENGL

#include "RenderCoreOpenGL.h"

struct SamplerStateGL
{
	Filter filter = Filter::Linear;
	TextureAddress textureAddress = TextureAddress::Clamp;

	bool operator==(const SamplerStateGL&) const = default;
};

SE_MAKE_HASHABLE(SamplerStateGL,
	t.filter,
	t.textureAddress
);

#endif // RENDER_OPENGL