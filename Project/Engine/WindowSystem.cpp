#include "stdafx.h"
#include "WindowSystem.h"
#include "Log.h"
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
	windowClassInfo.style         = CS_HREDRAW | CS_VREDRAW;
	windowClassInfo.lpfnWndProc   = WndProc;
	windowClassInfo.hInstance     = m_handleInstance;
	windowClassInfo.hIcon         = LoadIcon(nullptr, IDI_APPLICATION);
	windowClassInfo.hCursor       = LoadCursor(nullptr, IDC_ARROW);
	windowClassInfo.lpszClassName = windowClassName;
	if (!RegisterClassEx(&windowClassInfo))
	{
		Fatal("RegisterClassEx failed");
		return false;
	}

	const int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	const int screenHeight = GetSystemMetrics(SM_CYSCREEN);
	const int windowLeft = screenWidth / 2 - (int)createInfo.width / 2;
	const int windowTop = screenHeight / 2 - (int)createInfo.height / 2;

	RECT rect = { 0, 0, (int)createInfo.width, (int)createInfo.height };
	AdjustWindowRectEx(&rect, WS_OVERLAPPEDWINDOW, FALSE, WS_EX_OVERLAPPEDWINDOW);
	LONG width = rect.right - rect.left;
	LONG height = rect.bottom - rect.top;

	m_hwnd = CreateWindowEx(WS_EX_OVERLAPPEDWINDOW,
		windowClassName, L"Game",
		WS_OVERLAPPEDWINDOW,
		windowLeft, windowTop,
		width, height,
		nullptr, nullptr, m_handleInstance, nullptr);
	if (!m_hwnd)
	{
		Fatal("CreateWindowEx  failed");
		return false;
	}
	ShowWindow(m_hwnd, SW_SHOW);
	SetForegroundWindow(m_hwnd);
	SetFocus(m_hwnd);

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
	while (PeekMessage(&m_msg, 0, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&m_msg);
		DispatchMessage(&m_msg);
	}
}
//=============================================================================