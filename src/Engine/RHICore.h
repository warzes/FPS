﻿#pragma once

#include "HashCombine.h"

#pragma region [ Constant ]

// TODO: сделать общее для всех
#if RENDER_D3D11
constexpr const int RHI_BACKBUFFER_COUNT = 3;
#else
constexpr const int RHI_BACKBUFFER_COUNT = 2;
#endif

#pragma endregion

#pragma region [ Enum ]

enum class RenderFeature
{
	Raytracing
};

enum class PixelFormat
{
	R32Float,
	RG32Float,
	RGB32Float,
	RGBA32Float,
	R8UNorm,
	RG8UNorm,
	RGBA8UNorm,

#if RENDER_D3D11
	// TODO: ===> обновить под другие апи
	RGB10A2UNorm, // D3D11 support, D3D12/Vulkan/OpenGL unsupported
	BGRA8UNorm,   // D3D11, D3D12/Vulkan/OpenGL unsupported
#endif
};

enum class CullMode
{
	None,  // No culling
	Front, // Cull front-facing primitives
	Back,  // Cull back-facing primitives
};

enum class FrontFace
{
	Clockwise,
	CounterClockwise
};

enum class ComparisonFunc
{
	Always,         // comparison always succeeds
	Never,          // comparison always fails
	Less,           // passes if source is less than the destination
	Equal,          // passes if source is equal to the destination
	NotEqual,       // passes if source is not equal to the destination
	LessEqual,      // passes if source is less than or equal to the destination
	Greater,        // passes if source is greater than to the destination
	GreaterEqual,   // passes if source is greater than or equal to the destination
};

enum class StencilOp
{
	Keep,                // Does not update the stencil buffer entry.
	Zero,                // Sets the stencil buffer entry to 0.
	Replace,             // Replaces the stencil buffer entry with a reference value.
	Increment,           // Increments the stencil buffer entry, wrapping to 0 if the new value exceeds the maximum value.
	Decrement,           // Decrements the stencil buffer entry, wrapping to the maximum value if the new value is less than 0.
	IncrementSaturation, // Increments the stencil buffer entry, clamping to the maximum value.
	DecrementSaturation, // Decrements the stencil buffer entry, clamping to 0.
	Invert               // Inverts the bits in the stencil buffer entry.
};

enum class Blend
{
	One, // Each component of the color is multiplied by {1, 1, 1, 1}.
	Zero, // Each component of the color is multiplied by {0, 0, 0, 0}.
	SrcColor, // Each component of the color is multiplied by the source color. {Rs, Gs, Bs, As}, where Rs, Gs, Bs, As are color source values.		
	InvSrcColor, // Each component of the color is multiplied by the inverse of the source color. {1 - Rs, 1 - Gs, 1 - Bs, 1 - As}, where Rs, Gs, Bs, As are color source values.
	SrcAlpha, // Each component of the color is multiplied by the alpha value of the source. {As, As, As, As}, where As is the source alpha value.		
	InvSrcAlpha, // Each component of the color is multiplied by the inverse of the alpha value of the source. {1 - As, 1 - As, 1 - As, 1 - As}, where As is the source alpha value.
	DstColor, // Each component color is multiplied by the destination color. {Rd, Gd, Bd, Ad}, where Rd, Gd, Bd, Ad are color destination values.
	InvDstColor, // Each component of the color is multiplied by the inversed destination color. {1 - Rd, 1 - Gd, 1 - Bd, 1 - Ad}, where Rd, Gd, Bd, Ad are color destination values.
	DstAlpha, // Each component of the color is multiplied by the alpha value of the destination. {Ad, Ad, Ad, Ad}, where Ad is the destination alpha value.
	InvDstAlpha, // Each component of the color is multiplied by the inversed alpha value of the destination. {1 - Ad, 1 - Ad, 1 - Ad, 1 - Ad}, where Ad is the destination alpha value.
	//	BlendFactor, // Each component of the color is multiplied by a constant in the BlendFactor
	//	InverseBlendFactor, // Each component of the color is multiplied by a inversed constant in the BlendFactor
	//	SourceAlphaSaturation // Each component of the color is multiplied by either the alpha of the source color, or the inverse of the alpha of the source color, whichever is greater. {f, f, f, 1}, where f = min(As, 1 - As), where As is the source alpha value.
};

enum class BlendFunction
{
	Add,             // The function will adds destination to the source. (srcColor * srcBlend) + (destColor * destBlend)	
	Subtract,        // The function will subtracts destination from source. (srcColor * srcBlend) - (destColor * destBlend)
	ReverseSubtract, // The function will subtracts source from destination. (destColor * destBlend) - (srcColor * srcBlend) 
	Min,             // The function will extracts minimum of the source and destination. min((srcColor * srcBlend),(destColor * destBlend))
	Max              // The function will extracts maximum of the source and destination. max((srcColor * srcBlend),(destColor * destBlend))
};

enum class Topology
{
	PointList,
	LineList,
	LineStrip,
	TriangleList,
	TriangleStrip
};

enum class TopologyKind
{
	Points,
	Lines,
	Triangles
};

enum class VertexFormat
{
	Float1,
	Float2,
	Float3,
	Float4,
	UChar1,
	UChar2,
	UChar4,
	UChar1Normalized,
	UChar2Normalized,
	UChar4Normalized,
};

enum class ShaderStage
{
	Vertex,
	Fragment,
	Raygen,
	Miss,
	ClosestHit
};

enum class Filter
{
	Linear,
	Nearest,

#if RENDER_D3D11
	Anisotropic, // D3D11 support, D3D12/Vulkan/OpenGL unsupported
	LinearPoint, // D3D11 support, D3D12/Vulkan/OpenGL unsupported
	ComparisonLinearPoint // D3D11 support, D3D12/Vulkan/OpenGL unsupported
#endif
};

enum class TextureAddress
{
	Wrap,  // Texels outside range will form the tile at every integer junction.		
	Clamp, // Texels outside range will be set to color of 0.0 or 1.0 texel.
	MirrorWrap,

#if RENDER_D3D11
	Border, // D3D11 support, D3D12/Vulkan/OpenGL unsupported
#endif
};

#pragma endregion

#pragma region [ Struct ]

struct DepthBias final
{
	bool operator==(const DepthBias&) const = default;

	float factor{ 0.0f };
	float units{ 0.0f };
};

SE_MAKE_HASHABLE(DepthBias,
	t.factor,
	t.units
);

struct RasterizerState final
{
	bool operator==(const RasterizerState&) const = default;

	CullMode                 cullMode{ CullMode::None };
	FrontFace                frontFace{ FrontFace::Clockwise };
	bool                     depthClipEnable{ true };
	bool                     scissorEnabled{ false };
	std::optional<DepthBias> depthBias;
};

SE_MAKE_HASHABLE(RasterizerState,
	t.cullMode,
	t.frontFace,
	t.depthClipEnable,
	t.scissorEnabled,
	t.depthBias
);

struct SamplerState final
{
	bool operator==(const SamplerState&) const = default;

	Filter         filter{ Filter::Linear };
	TextureAddress textureAddress{ TextureAddress::Clamp };
	float          mipLODBias{ 0.0f };
	uint32_t       maxAnisotropy{ 1 };
	ComparisonFunc comparisonFunc{ ComparisonFunc::Never };
	float          borderColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	float          minLOD{ -FLT_MAX };
	float          maxLOD{ FLT_MAX };
};

SE_MAKE_HASHABLE(SamplerState,
	t.filter,
	t.textureAddress,
	t.mipLODBias,
	t.maxAnisotropy,
	t.comparisonFunc,
	t.borderColor[0],
	t.borderColor[1],
	t.borderColor[2],
	t.borderColor[3],
	t.minLOD,
	t.maxLOD
);


#pragma endregion



#pragma region [ OLD ]



struct Viewport final
{
	glm::vec2 position = { 0.0f, 0.0f };
	glm::vec2 size = { 0.0f, 0.0f };
	float     minDepth = 0.0f;
	float     maxDepth = 1.0f;

	bool operator==(const Viewport&) const = default;
};

struct Scissor final
{
	glm::vec2 position = { 0.0f, 0.0f };
	glm::vec2 size = { 0.0f, 0.0f };

	bool operator==(const Scissor&) const = default;
};

struct DepthMode final
{
	DepthMode() = default;
	DepthMode(ComparisonFunc _func) : func(_func) {}

	bool           writeMask = true;
	ComparisonFunc func = ComparisonFunc::Always;

	bool operator==(const DepthMode&) const = default;
};

struct StencilMode final
{
	uint8_t        readMask = 255;
	uint8_t        writeMask = 255;

	StencilOp      depthFailOp = StencilOp::Keep;
	StencilOp      failOp = StencilOp::Keep;
	ComparisonFunc func = ComparisonFunc::Always;
	StencilOp      passOp = StencilOp::Keep;

	uint8_t        reference = 1;

	bool operator==(const StencilMode&) const = default;
};


struct ColorMask final
{
	bool red = true;
	bool green = true;
	bool blue = true;
	bool alpha = true;

	bool operator==(const ColorMask&) const = default;
};

struct BlendMode final
{
	constexpr BlendMode(Blend color_src, Blend color_dst, Blend alpha_src, Blend alpha_dst)
		: colorSrc(color_src), colorDst(color_dst), alphaSrc(alpha_src), alphaDst(alpha_dst) {
	}
	constexpr BlendMode(Blend src, Blend dst) : BlendMode(src, dst, src, dst) {}

	BlendFunction colorFunc = BlendFunction::Add;
	Blend         colorSrc;
	Blend         colorDst;

	BlendFunction alphaFunc = BlendFunction::Add;
	Blend         alphaSrc;
	Blend         alphaDst;

	ColorMask     colorMask;

	bool operator==(const BlendMode&) const = default;
};

namespace BlendStates
{
	constexpr BlendMode Opaque = BlendMode(Blend::One, Blend::Zero);
	constexpr BlendMode AlphaBlend = BlendMode(Blend::One, Blend::InvSrcAlpha);
	constexpr BlendMode Additive = BlendMode(Blend::SrcAlpha, Blend::One);
	constexpr BlendMode NonPremultiplied = BlendMode(Blend::SrcAlpha, Blend::InvSrcAlpha);
}

struct InputLayout final
{
	enum class Rate
	{
		Vertex,
		Instance
	};

	struct Attribute final
	{
		Attribute(VertexFormat format, size_t offset);

		VertexFormat format;
		size_t       offset;

		bool operator==(const Attribute&) const = default;
	};

	InputLayout(Rate rate, std::unordered_map<uint32_t, Attribute> attributes);
	InputLayout(Rate rate, const std::vector<Attribute>& attributes);

	Rate                                    rate;
	std::unordered_map<uint32_t, Attribute> attributes;

	bool operator==(const InputLayout&) const = default;
};

SE_MAKE_HASHABLE(InputLayout::Attribute,
	t.format,
	t.offset
);

SE_MAKE_HASHABLE(InputLayout,
	t.attributes,
	t.rate
);

SE_MAKE_HASHABLE(ColorMask,
	t.red,
	t.green,
	t.blue,
	t.alpha
);

SE_MAKE_HASHABLE(BlendMode,
	t.alphaFunc,
	t.alphaDst,
	t.alphaSrc,
	t.colorFunc,
	t.colorDst,
	t.colorSrc,
	t.colorMask
);

SE_MAKE_HASHABLE(DepthMode,
	t.writeMask,
	t.func
);

SE_MAKE_HASHABLE(StencilMode,
	t.readMask,
	t.writeMask,
	t.depthFailOp,
	t.failOp,
	t.func,
	t.passOp
);



SE_MAKE_HASHABLE(std::vector<InputLayout>, t);

TopologyKind GetTopologyKind(Topology topology);
uint32_t     GetFormatChannelsCount(PixelFormat format);
uint32_t     GetFormatChannelSize(PixelFormat format);
uint32_t     GetMipCount(uint32_t width, uint32_t height);
uint32_t     GetMipWidth(uint32_t base_width, uint32_t mip_level);
uint32_t     GetMipHeight(uint32_t base_height, uint32_t mip_level);

template<typename T>
inline T AlignUp(T size, size_t alignment) noexcept
{
	if (alignment > 0)
	{
		assert(((alignment - 1) & alignment) == 0);
		auto mask = static_cast<T>(alignment - 1);
		return (size + mask) & ~mask;
	}
	return size;
}

using ShaderHandle = struct ShaderHandle;
using TextureHandle = struct TextureHandle;
using RenderTargetHandle = struct RenderTargetHandle;
using VertexBufferHandle = struct VertexBufferHandle;
using IndexBufferHandle = struct IndexBufferHandle;
using UniformBufferHandle = struct UniformBufferHandle;
#if RENDER_VULKAN
using RaytracingShaderHandle = struct RaytracingShaderHandle;
using StorageBufferHandle = struct StorageBufferHandle;
using BottomLevelAccelerationStructureHandle = struct BottomLevelAccelerationStructureHandle;
using TopLevelAccelerationStructureHandle = struct TopLevelAccelerationStructureHandle;
#endif // RENDER_VULKAN

class Noncopyable
{
protected:
	Noncopyable() = default;

private:
	Noncopyable(const Noncopyable&) = delete;
	Noncopyable& operator=(const Noncopyable&) = delete;
};

struct RenderSystemCreateInfo final
{
	std::unordered_set<RenderFeature> features{};
	bool                              vsync{ false };
};

#pragma endregion