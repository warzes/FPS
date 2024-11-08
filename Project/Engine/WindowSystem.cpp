#include "stdafx.h"
#include "WindowSystem.h"
#include "Log.h"
#include "RenderSystem.h"
#include "InputSystem.h"
//=============================================================================
extern void RequestExit();
extern RenderSystem* thisRenderSystem;
extern InputSystem* thisInputSystem;
//=============================================================================
namespace
{
	constexpr auto windowClassName = L"Sapphire Window Class";
}
WindowSystem* thisWindowSystem = nullptr;

//=============================================================================
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
{
	static bool s_minimized = false;
	static bool s_in_suspend = false;
	static bool s_in_sizemove = false;
	static bool s_fullscreen = false;
	static int restoreWidth = 1024;
	static int restoreHeight = 768;

	if (thisInputSystem) thisInputSystem->proc(msg, wParam, lParam);

	switch (msg)
	{
	case WM_DESTROY:
		RequestExit();
		PostQuitMessage(0);
		break;
	case WM_DISPLAYCHANGE:
		break;

	case WM_MOVE:
		break;

	case WM_SIZE:
		if (wParam == SIZE_MINIMIZED)
		{
			if (!s_minimized)
			{
				s_minimized = true;
				//if (!s_in_suspend) OnSuspending();
				s_in_suspend = true;
			}
		}
		else if (s_minimized)
		{
			s_minimized = false;
			//if (s_in_suspend) OnResuming();
			s_in_suspend = false;
		}
		else if (!s_in_sizemove)
		{
			if (thisWindowSystem)
			{
				thisWindowSystem->m_windowWidth = LOWORD(lParam);
				thisWindowSystem->m_windowHeight = HIWORD(lParam);
			}
			if (thisRenderSystem) thisRenderSystem->Resize(LOWORD(lParam), HIWORD(lParam));
		}
		break;
	case WM_ENTERSIZEMOVE:
		s_in_sizemove = true;
		break;
	case WM_EXITSIZEMOVE:
		s_in_sizemove = false;
		{
			RECT rc;
			GetClientRect(hWnd, &rc);
			if (thisWindowSystem)
			{
				thisWindowSystem->m_windowWidth = rc.right - rc.left;
				thisWindowSystem->m_windowHeight = rc.bottom - rc.top;
			}
			if (thisRenderSystem) thisRenderSystem->Resize(rc.right - rc.left, rc.bottom - rc.top);
		}
		break;
	case WM_GETMINMAXINFO:
		if (lParam)
		{
			auto info = reinterpret_cast<MINMAXINFO*>(lParam);
			info->ptMinTrackSize.x = 320;
			info->ptMinTrackSize.y = 200;
		}
		break;
	case WM_ACTIVATEAPP:
		if (wParam)
		{
			//OnActivated();
		}
		else
		{
			//OnDeactivated();
		}
		break;
	case WM_POWERBROADCAST:
		switch (wParam)
		{
		case PBT_APMQUERYSUSPEND:
			//if (!s_in_suspend) OnSuspending();
			s_in_suspend = true;
			return TRUE;

		case PBT_APMRESUMESUSPEND:
			if (!s_minimized)
			{
				//if (s_in_suspend) OnResuming();
				s_in_suspend = false;
			}
			return TRUE;

		default:
			break;
		}
		break;
	case WM_SYSKEYDOWN:
		if (wParam == VK_RETURN && (lParam & 0x60000000) == 0x20000000)
		{
			// Implements the classic ALT+ENTER fullscreen toggle
			if (s_fullscreen)
			{
				SetWindowLongPtr(hWnd, GWL_STYLE, WS_OVERLAPPEDWINDOW);
				SetWindowLongPtr(hWnd, GWL_EXSTYLE, 0);

				ShowWindow(hWnd, SW_SHOWNORMAL);
				// TODO: вернуть старую позицию окна
				SetWindowPos(hWnd, HWND_TOP, 0, 0, restoreWidth, restoreHeight, SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED);
			}
			else
			{
				if (thisWindowSystem)
				{
					restoreWidth = thisWindowSystem->m_windowWidth;
					restoreHeight = thisWindowSystem->m_windowHeight;
				}

				SetWindowLongPtr(hWnd, GWL_STYLE, WS_POPUP);
				SetWindowLongPtr(hWnd, GWL_EXSTYLE, WS_EX_TOPMOST);

				SetWindowPos(hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

				ShowWindow(hWnd, SW_SHOWMAXIMIZED);
			}

			s_fullscreen = !s_fullscreen;
		}
		break;	

	case WM_MENUCHAR:
		// A menu is active and the user presses a key that does not correspond to any mnemonic or accelerator key. Ignore so we don't produce an error beep.
		return MAKELRESULT(0, MNC_CLOSE);
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
	windowClassInfo.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
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
	UpdateWindow(m_hwnd);

	GetClientRect(m_hwnd, &rect);
	m_windowWidth = static_cast<uint32_t>(rect.right - rect.left);
	m_windowHeight = static_cast<uint32_t>(rect.bottom - rect.top);

	thisWindowSystem = this;

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
	while (PeekMessage(&m_msg, nullptr, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&m_msg);
		DispatchMessage(&m_msg);
	}
}
//=============================================================================