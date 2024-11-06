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
