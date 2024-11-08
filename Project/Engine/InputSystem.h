#pragma once

#include "Key.h"

class InputSystem final
{
public:
	~InputSystem();

	bool Create();
	void Destroy();

	KeyState GetKetState(Key key);

private:
	friend LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM) noexcept;

	void proc(UINT msg, WPARAM wParam, LPARAM lParam);

	KeyState m_keyState[static_cast<size_t>(Key::Count)] = {};
};