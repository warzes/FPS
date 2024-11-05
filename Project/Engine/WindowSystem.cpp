#include "stdafx.h"
#include "WindowSystem.h"
//=============================================================================
void RequestExit();
//=============================================================================
namespace
{
	constexpr auto windowClassName = L"Sapphire Window Class";
}
//=============================================================================
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
{
	switch (msg)
	{
	case WM_DESTROY:
		RequestExit();
		DestroyWindow(hWnd);
		PostQuitMessage(0);
		break;
	default:
		break;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}
//=============================================================================
WindowSystem::~WindowSystem()
{
	assert(!m_hwnd);
}
//=============================================================================
bool WindowSystem::Create(const WindowSystemCreateInfo& createInfo)
{
	m_handleInstance = GetModuleHandle(nullptr);

	WNDCLASSEX windowClassInfo{ .cbSize = sizeof(WNDCLASSEX) };
	windowClassInfo.style = CS_HREDRAW | CS_VREDRAW;
	windowClassInfo.lpfnWndProc = WndProc;
	windowClassInfo.hInstance = m_handleInstance;
	windowClassInfo.hCursor = LoadCursor(nullptr, IDC_ARROW);
	windowClassInfo.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	windowClassInfo.lpszClassName = windowClassName;
	RegisterClassEx(&windowClassInfo);

	const int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	const int screenHeight = GetSystemMetrics(SM_CYSCREEN);
	const int windowLeft = screenWidth / 2 - (int)createInfo.width / 2;
	const int windowTop = screenHeight / 2 - (int)createInfo.height / 2;

	m_hwnd = CreateWindow(windowClassName, L"Game",
		WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
		windowLeft, windowTop,
		(int)createInfo.width, (int)createInfo.height,
		nullptr, nullptr, m_handleInstance, nullptr);
	ShowWindow(m_hwnd, SW_SHOW);
	SetForegroundWindow(m_hwnd);
	SetFocus(m_hwnd);

	RECT rect;
	GetClientRect(m_hwnd, &rect);
	m_windowWidth = static_cast<uint32_t>(rect.right - rect.left);
	m_windowHeight = static_cast<uint32_t>(rect.bottom - rect.top);

	return true;
}
//=============================================================================
void WindowSystem::Destroy()
{
	DestroyWindow(m_hwnd);
	m_hwnd = nullptr;
}
//=============================================================================
void WindowSystem::PollEvent()
{
	while (PeekMessage(&m_msg, m_hwnd, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&m_msg);
		DispatchMessage(&m_msg);
	}
}
//=============================================================================