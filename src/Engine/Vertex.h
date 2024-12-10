#pragma once

#include "RHICore.h"

namespace vertex
{
	namespace location
	{
		constexpr auto Position  = "POSITION_LOCATION";
		constexpr auto Color     = "COLOR_LOCATION";
		constexpr auto TexCoord  = "TEXCOORD_LOCATION";
		constexpr auto Normal    = "NORMAL_LOCATION";
		constexpr auto Tangent   = "TANGENT_LOCATION";
		constexpr auto Bitangent = "BITANGENT_LOCATION";
	}

	namespace defaults
	{
		constexpr glm::vec3 Position  = { 0.0f, 0.0f, 0.0f };
		constexpr glm::vec4 Color     = { 1.0f, 1.0f, 1.0f, 1.0f };
		constexpr glm::vec2 TexCoord  = { 0.0f, 0.0f };
		constexpr glm::vec3 Normal    = { 0.0f, 0.0f, 0.0f };
		constexpr glm::vec3 Tangent   = { 0.0f, 1.0f, 0.0f };
		constexpr glm::vec3 Bitangent = { 0.0f, 1.0f, 0.0f };
	}

	std::vector<std::string> MakeSequentialLocationDefines(const std::vector<std::string>& locations);

	struct Position final
	{
		glm::vec3 pos = defaults::Position;

		static const InputLayout Layout;

		static inline const auto Defines = MakeSequentialLocationDefines({
			location::Position
			});
	};

	struct PositionColor final
	{
		glm::vec3 pos = defaults::Position;
		glm::vec4 color = defaults::Color;

		static const InputLayout Layout;

		static inline const auto Defines = MakeSequentialLocationDefines({
			location::Position,
			location::Color
			});
	};

	struct PositionTexture final
	{
		glm::vec3 pos = defaults::Position;
		glm::vec2 texcoord = defaults::TexCoord;

		static const InputLayout Layout;

		static inline const auto Defines = MakeSequentialLocationDefines({
			location::Position,
			location::TexCoord
			});
	};

	struct PositionNormal final
	{
		glm::vec3 pos = defaults::Position;
		glm::vec3 normal = defaults::Normal;

		static const InputLayout Layout;

		static inline const auto Defines = MakeSequentialLocationDefines({
			location::Position,
			location::Normal
			});
	};

	struct PositionColorNormal final
	{
		glm::vec3 pos = defaults::Position;
		glm::vec4 color = defaults::Color;
		glm::vec3 normal = defaults::Normal;

		static const InputLayout Layout;

		static inline const auto Defines = MakeSequentialLocationDefines({
			location::Position,
			location::Color,
			location::Normal
			});
	};

	struct PositionColorTexture final
	{
		glm::vec3 pos = defaults::Position;
		glm::vec4 color = defaults::Color;
		glm::vec2 texcoord = defaults::TexCoord;

		static const InputLayout Layout;

		static inline const auto Defines = MakeSequentialLocationDefines({
			location::Position,
			location::Color,
			location::TexCoord
			});
	};

	struct PositionTextureNormal final
	{
		glm::vec3 pos = defaults::Position;
		glm::vec2 texcoord = defaults::TexCoord;
		glm::vec3 normal = defaults::Normal;

		static const InputLayout Layout;

		static inline const auto Defines = MakeSequentialLocationDefines({
			location::Position,
			location::TexCoord,
			location::Normal
			});
	};

	struct PositionColorTextureNormal final
	{
		glm::vec3 pos = defaults::Position;
		glm::vec4 color = defaults::Color;
		glm::vec2 texcoord = defaults::TexCoord;
		glm::vec3 normal = defaults::Normal;

		static const InputLayout Layout;

		static inline const auto Defines = MakeSequentialLocationDefines({
			location::Position,
			location::Color,
			location::TexCoord,
			location::Normal
			});
	};

	struct PositionColorTextureNormalTangent final
	{
		glm::vec3 pos = defaults::Position;
		glm::vec4 color = defaults::Color;
		glm::vec2 texcoord = defaults::TexCoord;
		glm::vec3 normal = defaults::Normal;
		glm::vec3 tangent = defaults::Tangent;

		static const InputLayout Layout;

		static inline const auto Defines = MakeSequentialLocationDefines({
			location::Position,
			location::Color,
			location::TexCoord,
			location::Normal,
			location::Tangent
			});
	};

	struct PositionColorTextureNormalTangentBitangent final
	{
		glm::vec3 pos = defaults::Position;
		glm::vec4 color = defaults::Color;
		glm::vec2 texcoord = defaults::TexCoord;
		glm::vec3 normal = defaults::Normal;
		glm::vec3 tangent = defaults::Tangent;
		glm::vec3 bitangent = defaults::Bitangent;

		static const InputLayout Layout;

		static inline const auto Defines = MakeSequentialLocationDefines({
			location::Position,
			location::Color,
			location::TexCoord,
			location::Normal,
			location::Tangent,
			location::Bitangent
			});
	};
}
