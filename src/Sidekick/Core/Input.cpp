#include "Sidekick/Core/Input.hpp"

#include "Sidekick/Core/Event.hpp"

#include <cassert>

namespace Sidekick::Core
{
std::bitset<MAX_KEYS> Input::s_KeyDown;
std::bitset<MAX_KEYS> Input::s_KeyPressed;
std::bitset<MAX_KEYS> Input::s_KeyReleased;

void Input::Init()
{
    s_KeyDown.reset();
    s_KeyPressed.reset();
    s_KeyReleased.reset();
}

void Input::OnKeyPressedEvent(KeyPressedEvent& event)
{
    int idx = static_cast<int>(event.GetKeyCode());
    assert(idx >= 0 && idx < MAX_KEYS);

    if (!s_KeyDown[idx])
        s_KeyPressed.set(idx);
    s_KeyDown.set(idx);
}

void Input::OnKeyReleasedEvent(KeyReleasedEvent& event)
{
    int idx = static_cast<int>(event.GetKeyCode());
    assert(idx >= 0 && idx < MAX_KEYS);

    if (s_KeyDown[idx])
        s_KeyReleased.set(idx);
    s_KeyDown.reset(idx);
}

void Input::Update()
{
    s_KeyPressed.reset();
    s_KeyReleased.reset();
}

bool Input::IsKeyDown(KeyCode key)
{
    return s_KeyDown[static_cast<int>(key)];
}
bool Input::IsKeyPressed(KeyCode key)
{
    return s_KeyPressed[static_cast<int>(key)];
}
bool Input::IsKeyReleased(KeyCode key)
{
    return s_KeyReleased[static_cast<int>(key)];
}
} // namespace Sidekick::Core
