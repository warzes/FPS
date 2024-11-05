#pragma once

struct WindowSystemCreateInfo final
{
	uint32_t width{ 1600 };
	uint32_t height{ 900 };
	bool resize = true;
};

class WindowSystem final
{
public:
	~WindowSystem();

	bool Create(const WindowSystemCreateInfo& createInfo);
	void Destroy();

	void PollEvent();

	HWND      GetWindowHWND() const { return m_hwnd; }
	HINSTANCE GetWindowInstance() const { return m_handleInstance; }
	uint32_t  GetWindowWidth() const { return m_windowWidth; }
	uint32_t  GetWindowHeight() const { return m_windowHeight; }

private:
	friend LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM) noexcept;

	HINSTANCE m_handleInstance{ nullptr };
	HWND      m_hwnd{ nullptr };
	MSG       m_msg{};
	uint32_t  m_windowWidth{ 0 };
	uint32_t  m_windowHeight{ 0 };

	bool      m_minimized{ false };
	bool      m_requestedExit{ false };
};