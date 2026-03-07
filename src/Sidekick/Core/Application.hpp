#pragma once

#include "Sidekick/Core/Window.hpp"

namespace Sidekick
{
struct Event;
struct WindowClosedEvent;
struct WindowResizeEvent;

class Application
{
public:
  Application();

  void Run();

private:
  void OnEvent(Event& event);
  bool OnWindowClosed(WindowClosedEvent& event);
  bool OnWindowResized(WindowResizeEvent& event);

  bool m_Running{true};
  bool m_IsMinimized{false};
  Window m_Window;
};
} // namespace Sidekick
