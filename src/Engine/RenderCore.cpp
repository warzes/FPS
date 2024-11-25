#include "stdafx.h"
#include "RenderCore.h"

InputLayout::Attribute::Attribute(VertexFormat _format, size_t _offset) :
	format(_format),
	offset(_offset)
{
}

InputLayout::InputLayout(Rate _rate, std::unordered_map<uint32_t, Attribute> _attributes) :
	rate(_rate),
	attributes(std::move(_attributes))
{
}

InputLayout::InputLayout(Rate _rate, const std::vector<Attribute>& _attributes) :
	rate(_rate)
{
	for (size_t i = 0; i < _attributes.size(); i++)
	{
		attributes.insert({ (uint32_t)i, _attributes.at(i) });
	}
}