#pragma once

inline void HashCombine(std::size_t&) {}

template <typename T, typename... Rest>
inline void HashCombine(std::size_t& seed, const T& value, Rest... rest)
{
	std::hash<T> hasher;
	seed ^= hasher(value) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	HashCombine(seed, rest...);
}

template <typename T, typename... Rest>
inline void HashCombine(std::size_t& seed, const std::vector<T>& values, Rest... rest)
{
	for (const auto& value : values)
	{
		HashCombine(seed, value);
	}
	HashCombine(seed, rest...);
}

template <typename T, typename U, typename... Rest>
inline void HashCombine(std::size_t& seed, const std::unordered_map<T, U>& values, Rest... rest)
{
	for (const auto& [key, value] : values)
	{
		HashCombine(seed, key);
		HashCombine(seed, value);
	}
	HashCombine(seed, rest...);
}

// TODO: сделать так чтобы хеш вычислялся в компайлтайме, и тогда бы здесь было одно число (а точнее это вообще было бы не нужно- можно было сразу хранить хеш)

#define SE_MAKE_HASHABLE(T, ...) \
	namespace std {\
		template<> struct hash<T> {\
			std::size_t operator()(const T& t) const {\
				std::size_t ret = 0;\
				HashCombine(ret, __VA_ARGS__);\
				return ret;\
			}\
		};\
	}
