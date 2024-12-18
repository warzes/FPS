#pragma once

#if defined(_WIN32)
std::string ConvertToUTF8(const std::wstring& wstr);
std::wstring ConvertToUTF16(const std::string& str);
#endif // _WIN32