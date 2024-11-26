#pragma once

#if RENDER_OPENGL

#include "RenderCore.h"

static const std::unordered_map<VertexFormat, GLint> VertexFormatSizeMap = {
	{ VertexFormat::Float1, 1 },
	{ VertexFormat::Float2, 2 },
	{ VertexFormat::Float3, 3 },
	{ VertexFormat::Float4, 4 },
	{ VertexFormat::UChar1, 1 },
	{ VertexFormat::UChar2, 2 },
	{ VertexFormat::UChar4, 4 },
	{ VertexFormat::UChar1Normalized, 1 },
	{ VertexFormat::UChar2Normalized, 2 },
	{ VertexFormat::UChar4Normalized, 4 }
};

static const std::unordered_map<VertexFormat, GLint> VertexFormatTypeMap = {
	{ VertexFormat::Float1, GL_FLOAT },
	{ VertexFormat::Float2, GL_FLOAT },
	{ VertexFormat::Float3, GL_FLOAT },
	{ VertexFormat::Float4, GL_FLOAT },
	{ VertexFormat::UChar1, GL_UNSIGNED_BYTE },
	{ VertexFormat::UChar2, GL_UNSIGNED_BYTE },
	{ VertexFormat::UChar4, GL_UNSIGNED_BYTE },
	{ VertexFormat::UChar1Normalized, GL_UNSIGNED_BYTE },
	{ VertexFormat::UChar2Normalized, GL_UNSIGNED_BYTE },
	{ VertexFormat::UChar4Normalized, GL_UNSIGNED_BYTE }
};

static const std::unordered_map<PixelFormat, GLenum> PixelFormatTypeMap = {
	{ PixelFormat::R32Float, GL_FLOAT },
	{ PixelFormat::RG32Float, GL_FLOAT },
	{ PixelFormat::RGB32Float, GL_FLOAT },
	{ PixelFormat::RGBA32Float, GL_FLOAT },
	{ PixelFormat::R8UNorm, GL_UNSIGNED_BYTE },
	{ PixelFormat::RG8UNorm, GL_UNSIGNED_BYTE },
	{ PixelFormat::RGBA8UNorm, GL_UNSIGNED_BYTE },
};

static const std::unordered_map<VertexFormat, GLboolean> VertexFormatNormalizeMap = {
	{ VertexFormat::Float1, GL_FALSE },
	{ VertexFormat::Float2, GL_FALSE },
	{ VertexFormat::Float3, GL_FALSE },
	{ VertexFormat::Float4, GL_FALSE },
	{ VertexFormat::UChar1, GL_FALSE },
	{ VertexFormat::UChar2, GL_FALSE },
	{ VertexFormat::UChar4, GL_FALSE },
	{ VertexFormat::UChar1Normalized, GL_TRUE },
	{ VertexFormat::UChar2Normalized, GL_TRUE },
	{ VertexFormat::UChar4Normalized, GL_TRUE }
};

static const std::unordered_map<ComparisonFunc, GLenum> ComparisonFuncMap = {
	{ ComparisonFunc::Always, GL_ALWAYS },
	{ ComparisonFunc::Never, GL_NEVER },
	{ ComparisonFunc::Less, GL_LESS },
	{ ComparisonFunc::Equal, GL_EQUAL },
	{ ComparisonFunc::NotEqual, GL_NOTEQUAL },
	{ ComparisonFunc::LessEqual, GL_LEQUAL },
	{ ComparisonFunc::Greater, GL_GREATER },
	{ ComparisonFunc::GreaterEqual, GL_GEQUAL }
};

static const std::unordered_map<PixelFormat, GLenum> TextureInternalFormatMap = {
#if PLATFORM_EMSCRIPTEN
	// webgl and gles in ios devices cannot correctly handle 32-bit floating textures
	// yes, we force webgl to use 16-bit floating textures for all platforms because of ios
	{ PixelFormat::R32Float, GL_R16F },
	{ PixelFormat::RG32Float, GL_RG16F },
	{ PixelFormat::RGB32Float, GL_RGB16F },
	{ PixelFormat::RGBA32Float, GL_RGBA16F },
#else
	{ PixelFormat::R32Float, GL_R32F },
	{ PixelFormat::RG32Float, GL_RG32F },
	{ PixelFormat::RGB32Float, GL_RGB32F },
	{ PixelFormat::RGBA32Float, GL_RGBA32F },
#endif
	{ PixelFormat::R8UNorm, GL_R8 },
	{ PixelFormat::RG8UNorm, GL_RG8 },
	{ PixelFormat::RGBA8UNorm, GL_RGBA8 }
};

static const std::unordered_map<PixelFormat, GLenum> TextureFormatMap = {
	{ PixelFormat::R32Float, GL_RED },
	{ PixelFormat::RG32Float, GL_RG },
	{ PixelFormat::RGB32Float, GL_RGB },
	{ PixelFormat::RGBA32Float, GL_RGBA },
	{ PixelFormat::R8UNorm, GL_RED },
	{ PixelFormat::RG8UNorm, GL_RG },
	{ PixelFormat::RGBA8UNorm, GL_RGBA },
};

#endif // RENDER_OPENGL