#include "stdafx.h"
#include "Vertex.h"
//=============================================================================
std::vector<std::string> vertex::MakeSequentialLocationDefines(const std::vector<std::string>& locations)
{
	std::vector<std::string> result;

	for (size_t i = 0; i < locations.size(); i++)
	{
		const auto& location = locations.at(i);
		result.push_back(location + " " + std::to_string(i));
	}

	return result;
}
//=============================================================================
const InputLayout vertex::Position::Layout = InputLayout(InputLayout::Rate::Vertex, {
	{ VertexFormat::Float3, offsetof(Position, pos) }
	});
//=============================================================================
const InputLayout vertex::PositionColor::Layout = InputLayout(InputLayout::Rate::Vertex, {
	{ VertexFormat::Float3, offsetof(PositionColor, pos) },
	{ VertexFormat::Float4, offsetof(PositionColor, color) }
	});
//=============================================================================
const InputLayout vertex::PositionTexture::Layout = InputLayout(InputLayout::Rate::Vertex, {
	{ VertexFormat::Float3, offsetof(PositionTexture, pos) },
	{ VertexFormat::Float2, offsetof(PositionTexture, texcoord) }
	});
//=============================================================================
const InputLayout vertex::PositionNormal::Layout = InputLayout(InputLayout::Rate::Vertex, {
	{ VertexFormat::Float3, offsetof(PositionNormal, pos) },
	{ VertexFormat::Float3, offsetof(PositionNormal, normal) }
	});
//=============================================================================
const InputLayout vertex::PositionColorNormal::Layout = InputLayout(InputLayout::Rate::Vertex, {
	{ VertexFormat::Float3, offsetof(PositionColorNormal, pos) },
	{ VertexFormat::Float4, offsetof(PositionColorNormal, color) },
	{ VertexFormat::Float3, offsetof(PositionColorNormal, normal) }
	});
//=============================================================================
const InputLayout vertex::PositionColorTexture::Layout = InputLayout(InputLayout::Rate::Vertex, {
	{ VertexFormat::Float3, offsetof(PositionColorTexture, pos) },
	{ VertexFormat::Float4, offsetof(PositionColorTexture, color) },
	{ VertexFormat::Float2, offsetof(PositionColorTexture, texcoord) }
	});
//=============================================================================
const InputLayout vertex::PositionTextureNormal::Layout = InputLayout(InputLayout::Rate::Vertex, {
	{ VertexFormat::Float3, offsetof(PositionTextureNormal, pos) },
	{ VertexFormat::Float2, offsetof(PositionTextureNormal, texcoord) },
	{ VertexFormat::Float3, offsetof(PositionTextureNormal, normal) }
	});
//=============================================================================
const InputLayout vertex::PositionColorTextureNormal::Layout = InputLayout(InputLayout::Rate::Vertex, {
	{ VertexFormat::Float3, offsetof(PositionColorTextureNormal, pos) },
	{ VertexFormat::Float4, offsetof(PositionColorTextureNormal, color) },
	{ VertexFormat::Float2, offsetof(PositionColorTextureNormal, texcoord) },
	{ VertexFormat::Float3, offsetof(PositionColorTextureNormal, normal) }
	});
//=============================================================================
const InputLayout vertex::PositionColorTextureNormalTangent::Layout = InputLayout(InputLayout::Rate::Vertex, {
	{ VertexFormat::Float3, offsetof(PositionColorTextureNormalTangent, pos) },
	{ VertexFormat::Float4, offsetof(PositionColorTextureNormalTangent, color) },
	{ VertexFormat::Float2, offsetof(PositionColorTextureNormalTangent, texcoord) },
	{ VertexFormat::Float3, offsetof(PositionColorTextureNormalTangent, normal) },
	{ VertexFormat::Float3, offsetof(PositionColorTextureNormalTangent, tangent) }
	});
//=============================================================================
const InputLayout vertex::PositionColorTextureNormalTangentBitangent::Layout = InputLayout(InputLayout::Rate::Vertex, {
	{ VertexFormat::Float3, offsetof(PositionColorTextureNormalTangentBitangent, pos) },
	{ VertexFormat::Float4, offsetof(PositionColorTextureNormalTangentBitangent, color) },
	{ VertexFormat::Float2, offsetof(PositionColorTextureNormalTangentBitangent, texcoord) },
	{ VertexFormat::Float3, offsetof(PositionColorTextureNormalTangentBitangent, normal) },
	{ VertexFormat::Float3, offsetof(PositionColorTextureNormalTangentBitangent, tangent) },
	{ VertexFormat::Float3, offsetof(PositionColorTextureNormalTangentBitangent, bitangent) }
	});
//=============================================================================