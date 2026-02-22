#pragma once

#include "Sidekick/Core/Event.h"
#include "Sidekick/Core/KeyCodes.h"
#include "Sidekick/Core/MouseCodes.h"

namespace Sidekick::Core {

class Input {
 public:
  static void Initialize();
  static void Shutdown();
  static void BeginFrame();

  static void OnEvent(Event& event);

  static bool IsKeyPressed(KeyCode key_code);
  static bool IsMouseButtonPressed(MouseButton button);

  static double GetMouseX();
  static double GetMouseY();
  static double GetMouseDeltaX();
  static double GetMouseDeltaY();
};

}  // namespace Sidekick::Core
