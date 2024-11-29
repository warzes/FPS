#include "stdafx.h"
#if PLATFORM_WINDOWS
#include "InputSystem.h"
#include "WindowSystem.h"
//=============================================================================
InputSystem::~InputSystem()
{
	assert(!m_window);
}
//=============================================================================
bool InputSystem::Create(const InputSystemCreateInfo& createInfo)
{
	if (!m_keyboard.Create()) return false;
	m_keys.Reset();
	
	assert(m_window);
	if (!m_mouse.Create(m_window->GetHWND())) return false;

	return true;
}
//=============================================================================
void InputSystem::Destroy()
{
	m_mouse.Destroy();
	m_keyboard.Destroy();
	m_window = nullptr;
}
//=============================================================================
void InputSystem::ConnectWindowSystem(WindowSystem* window)
{
	m_window = window;
}
//=============================================================================
void InputSystem::Update()
{
	m_lastKeyboardState = m_keyboard.GetState();
	m_keys.Update(m_lastKeyboardState);
	m_lastBufferedMouseState = m_lastMouseState;
	m_lastMouseState = m_mouse.GetState(); // TODO: тут делается копия стейта, надо переделать
	m_mouseButtons.Update(m_lastMouseState);
}
//=============================================================================
void InputSystem::ProcessMessage(UINT message, WPARAM wParam, LPARAM lParam)
{
	m_keyboard.processMessage(message, wParam, lParam);
	m_mouse.processMessage(message, wParam, lParam);
}
//=============================================================================
bool InputSystem::IsKeyPress(Key key) const
{
	return m_lastKeyboardState.IsKeyDown(key);
}
//=============================================================================
glm::vec2 InputSystem::GetDeltaMouse() const
{
	return 
	{
		static_cast<float>(m_lastMouseState.x) - static_cast<float>(m_lastBufferedMouseState.x),
		static_cast<float>(m_lastMouseState.y) - static_cast<float>(m_lastBufferedMouseState.y),
	};
}
//=============================================================================
void InputSystem::onResuming()
{
	m_keys.Reset();
	m_mouseButtons.Reset();
}
//=============================================================================
void InputSystem::onActivated()
{
	m_keys.Reset();
	m_mouseButtons.Reset();
}
//=============================================================================
#endif // PLATFORM_WINDOWS