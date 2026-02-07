#pragma once

#include <bitset>

#include "Sidekick/Core/KeyCode.hpp"

namespace Sidekick::Core
{
constexpr int MAX_KEYS = 512;

class KeyPressedEvent;
class KeyReleasedEvent;
class Input
{
  public:
    static void Init();

    static void OnKeyPressedEvent(KeyPressedEvent& event);
    static void OnKeyReleasedEvent(KeyReleasedEvent& event);
    static void Update();

    static bool IsKeyDown(KeyCode key);
    static bool IsKeyPressed(KeyCode key);
    static bool IsKeyReleased(KeyCode key);

  private:
    static std::bitset<MAX_KEYS> s_KeyDown;
    static std::bitset<MAX_KEYS> s_KeyPressed;
    static std::bitset<MAX_KEYS> s_KeyReleased;
};
} // namespace Sidekick::Core
