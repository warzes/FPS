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

	KeyState m_keyState[static_cast<size_t>(Key::Count)] = {};
};