#pragma once

#include "InputStructs.h"

class KeyboardStateTracker final
{
public:
	KeyboardStateTracker() noexcept { Reset(); }

	void __cdecl Update(const KeyState& state) noexcept;

	void __cdecl Reset() noexcept;

	bool __cdecl IsKeyPressed(Key key) const noexcept { return pressed.IsKeyDown(key); }
	bool __cdecl IsKeyReleased(Key key) const noexcept { return released.IsKeyDown(key); }

	KeyState __cdecl GetLastState() const noexcept { return lastState; }

	KeyState released;
	KeyState pressed;
	KeyState lastState;
};

class Keyboard final
{
	friend class InputSystem;
public:
	[[nodiscard]] bool Create();
	void Destroy();

	void Reset();

	const KeyState& GetState() const { return m_state; }

private:
#if PLATFORM_WINDOWS
	void processMessage(UINT message, WPARAM wParam, LPARAM lParam);
#endif
	
	KeyState m_state{};
};