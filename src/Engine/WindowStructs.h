﻿#pragma once

struct WindowData final
{
#if PLATFORM_WINDOWS
	HINSTANCE handleInstance{ nullptr };
	HWND      hwnd{ nullptr };
#endif //PLATFORM_WINDOWS
	uint32_t  width{ 1600 };
	uint32_t  height{ 900 };
};

struct WindowSystemCreateInfo final
{
	std::string_view title{ "Game" };
	uint32_t         width{ 1600 };
	uint32_t         height{ 900 };
	bool             maximize{ true };
	bool             resize{ true };
	bool             fullScreen{ false };
};