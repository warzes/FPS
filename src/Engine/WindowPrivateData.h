#pragma once

struct WindowPrivateData final
{
#if PLATFORM_WINDOWS
	HINSTANCE handleInstance{ nullptr };
	HWND      hwnd{ nullptr };
#endif //PLATFORM_WINDOWS
	uint32_t  width{ 1600 };
	uint32_t  height{ 900 };
};