#pragma once

inline std::string ToString(const std::wstring& wide)
{
	std::string str(wide.length(), 0);
	std::transform(wide.begin(), wide.end(), str.begin(), [](wchar_t c) { return (char)c; });
	return str;
}