#pragma once

class Color final
{
public:
	static constexpr Color Red() { return Color(255, 0, 0, 255); }
	static constexpr Color Green() { return Color(0, 255, 0, 255); }
	static constexpr Color Blue() { return Color(0, 0, 255, 255); }
	static constexpr Color Black() { return Color(0, 0, 0, 255); }
	static constexpr Color White() { return Color(255, 255, 255, 255); }
	static constexpr Color Yellow() { return Color(255, 255, 0, 255); }
	static constexpr Color Purple() { return Color(255, 0, 255, 255); }
	static constexpr Color Cyan() { return Color(0, 255, 255, 255); }
	static constexpr Color Transparent() { return Color(0, 0, 0, 0); }
	static constexpr Color Gray() { return Color(127, 127, 127, 255); }
	static constexpr Color Ghost() { return Color(127, 127, 127, 127); }
	static constexpr Color Booger() { return Color(127, 127, 127, 200); }
	static constexpr Color Shadow() { return Color(0, 0, 0, 100); }

	constexpr Color(uint8_t red = 0, uint8_t green = 0, uint8_t blue = 0, uint8_t alpha = 255) : r(red), g(green), b(blue), a(alpha) {}
	constexpr Color(uint32_t rgba)
	{
		r = (rgba >> 0) & 0xFF;
		g = (rgba >> 8) & 0xFF;
		b = (rgba >> 16) & 0xFF;
		a = (rgba >> 24) & 0xFF;
	}

	constexpr glm::vec3 ToFloat3() const
	{
		return { 
			static_cast<float>(r) / 255.0f, 
			static_cast<float>(g) / 255.0f, 
			static_cast<float>(b) / 255.0f };
	}
	constexpr glm::vec4 ToFloat4() const
	{
		return {
			static_cast<float>(r) / 255.0f,
			static_cast<float>(g) / 255.0f,
			static_cast<float>(b) / 255.0f,
			static_cast<float>(a) / 255.0f,
		};
	}

	constexpr operator glm::vec3() const { return ToFloat3(); }
	constexpr operator glm::vec4() const { return ToFloat4(); }
	constexpr operator uint32_t() const 
	{ 
		uint32_t rgba = (uint32_t(r) | (uint32_t(g) << 8) | (uint32_t(b) << 16) | (uint32_t(a) << 24));
		return rgba;
	}

	static constexpr Color FromFloat3(const glm::vec3& value)
	{
		return { (uint8_t)(value.x * 255), (uint8_t)(value.y * 255), (uint8_t)(value.z * 255) };
	}

	static constexpr Color FromFloat4(const glm::vec4& value)
	{
		return { (uint8_t)(value.x * 255), (uint8_t)(value.y * 255), (uint8_t)(value.z * 255), (uint8_t)(value.w * 255) };
	}

	static constexpr Color Lerp(Color a, Color b, float i)
	{
		return FromFloat4(glm::lerp(a.ToFloat4(), b.ToFloat4(), i));
	}
	
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;
};