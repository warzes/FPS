#pragma once

#include "WindowPrivateData.h"

struct WindowSystemCreateInfo final
{
	std::string_view title{ "Game" };
	uint32_t         width{ 1600 };
	uint32_t         height{ 900 };
	bool             maximize{ true };
	bool             resize{ true };
	bool             fullscreen{ false };
};

class WindowSystem final
{
public:
	~WindowSystem();

	bool Create(const WindowSystemCreateInfo& createInfo);
	void Destroy();

	bool IsShouldClose() const;
	void PollEvent();

	uint32_t GetWidth() const { assert(m_width); return m_width; }
	uint32_t GetHeight() const { assert(m_height); return m_height; }
	
	uint32_t GetPositionX() const;
	uint32_t GetPositionY() const;

#if PLATFORM_WINDOWS
	HWND      GetHWND() const { return m_hwnd; }
	HINSTANCE GetWindowInstance() const { return m_handleInstance; }
#endif // PLATFORM_WINDOWS

private:
	void displayChange();
	void windowSizeChanged(uint32_t width, uint32_t height);
	void suspending();
	void resuming();
	void activated();
	void deactivated();

#if PLATFORM_WINDOWS
	friend LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM) noexcept;

	HINSTANCE m_handleInstance{ nullptr };
	HWND      m_hwnd{ nullptr };
	MSG       m_msg{};
	uint32_t  m_width{ 1600 };
	uint32_t  m_height{ 900 };
	bool      m_minimized{ false };
	bool      m_inSizemove{ false };
	bool      m_inSuspend{ false };
	bool      m_fullscreen{ false };
	bool      m_requestClose{ true };
#endif // PLATFORM_WINDOWS
};