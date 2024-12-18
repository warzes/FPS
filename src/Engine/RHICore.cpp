﻿#include "stdafx.h"
#include "RHICore.h"
//=============================================================================
InputLayout::Attribute::Attribute(VertexFormat _format, size_t _offset)
	: format(_format)
	, offset(_offset)
{
}
//=============================================================================
InputLayout::InputLayout(Rate _rate, std::unordered_map<uint32_t, Attribute> _attributes)
	: rate(_rate)
	, attributes(std::move(_attributes))
{
}
//=============================================================================
InputLayout::InputLayout(Rate _rate, const std::vector<Attribute>& _attributes)
	: rate(_rate)
{
	for (size_t i = 0; i < _attributes.size(); i++)
	{
		attributes.insert({ (uint32_t)i, _attributes.at(i) });
	}
}
//=============================================================================
TopologyKind GetTopologyKind(Topology topology)
{
	static const std::unordered_map<Topology, TopologyKind> TopologyKindMap = {
		{ Topology::PointList, TopologyKind::Points },
		{ Topology::LineList, TopologyKind::Lines},
		{ Topology::LineStrip, TopologyKind::Lines },
		{ Topology::TriangleList, TopologyKind::Triangles },
		{ Topology::TriangleStrip, TopologyKind::Triangles }
	};
	return TopologyKindMap.at(topology);
}
//=============================================================================
uint32_t GetFormatChannelsCount(PixelFormat format)
{
	static const std::unordered_map<PixelFormat, uint32_t> FormatChannelsMap = {
		{ PixelFormat::R32Float, 1 },
		{ PixelFormat::RG32Float, 2 },
		{ PixelFormat::RGB32Float, 3 },
		{ PixelFormat::RGBA32Float, 4 },
		{ PixelFormat::R8UNorm, 1 },
		{ PixelFormat::RG8UNorm, 2 },
		{ PixelFormat::RGBA8UNorm, 4 }
	};
	return FormatChannelsMap.at(format);
}
//=============================================================================
uint32_t GetFormatChannelSize(PixelFormat format)
{
	static const std::unordered_map<PixelFormat, uint32_t> FormatChannelSizeMap = {
		{ PixelFormat::R32Float, 4 },
		{ PixelFormat::RG32Float, 4 },
		{ PixelFormat::RGB32Float, 4 },
		{ PixelFormat::RGBA32Float, 4 },
		{ PixelFormat::R8UNorm, 1 },
		{ PixelFormat::RG8UNorm, 1 },
		{ PixelFormat::RGBA8UNorm, 1 }
	};
	return FormatChannelSizeMap.at(format);
}
//=============================================================================
uint32_t GetMipCount(uint32_t width, uint32_t height)
{
	return static_cast<uint32_t>(glm::floor(glm::log2(glm::max(width, height)))) + 1;
}
//=============================================================================
uint32_t GetMipWidth(uint32_t base_width, uint32_t mip_level)
{
	return glm::max<uint32_t>(1, static_cast<uint32_t>(glm::floor<uint32_t>(base_width >> mip_level)));
}
//=============================================================================
uint32_t GetMipHeight(uint32_t base_height, uint32_t mip_level)
{
	return glm::max<uint32_t>(1, static_cast<uint32_t>(glm::floor<uint32_t>(base_height >> mip_level)));
}
//=============================================================================