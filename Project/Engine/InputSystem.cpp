#include "stdafx.h"
#include "InputSystem.h"

InputSystem* thisInputSystem = nullptr;

InputSystem::~InputSystem()
{
	assert(!thisInputSystem);
}

bool InputSystem::Create()
{
	assert(!thisInputSystem);

	for (size_t i = 0; i < static_cast<size_t>(Key::Count); i++)
		m_keyState[i] = KeyState::Up;

	thisInputSystem = this;

	return true;
}

void InputSystem::Destroy()
{
	thisInputSystem = nullptr;
}

KeyState InputSystem::GetKetState(Key key)
{
	return m_keyState[static_cast<size_t>(key)];
}

void InputSystem::proc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_KEYDOWN:
	case WM_KEYUP:
		{
			bool isDown = (msg == WM_KEYDOWN);
			if (wParam == 'W')
				thisInputSystem->m_keyState[static_cast<size_t>(Key::KEY_W)] = isDown ? KeyState::Down : KeyState::Up;
			else if (wParam == 'A')
				thisInputSystem->m_keyState[static_cast<size_t>(Key::KEY_A)] = isDown ? KeyState::Down : KeyState::Up;
			else if (wParam == 'S')
				thisInputSystem->m_keyState[static_cast<size_t>(Key::KEY_S)] = isDown ? KeyState::Down : KeyState::Up;
			else if (wParam == 'D')
				thisInputSystem->m_keyState[static_cast<size_t>(Key::KEY_D)] = isDown ? KeyState::Down : KeyState::Up;
			else if (wParam == VK_UP)
				thisInputSystem->m_keyState[static_cast<size_t>(Key::KEY_UP)] = isDown ? KeyState::Down : KeyState::Up;
			else if (wParam == VK_LEFT)
				thisInputSystem->m_keyState[static_cast<size_t>(Key::KEY_LEFT)] = isDown ? KeyState::Down : KeyState::Up;
			else if (wParam == VK_DOWN)
				thisInputSystem->m_keyState[static_cast<size_t>(Key::KEY_DOWN)] = isDown ? KeyState::Down : KeyState::Up;
			else if (wParam == VK_RIGHT)
				thisInputSystem->m_keyState[static_cast<size_t>(Key::KEY_RIGHT)] = isDown ? KeyState::Down : KeyState::Up;
			break;
		}
	}
}
