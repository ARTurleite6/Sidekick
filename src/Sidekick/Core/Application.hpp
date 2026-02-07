#pragma once

#include <glad/glad.h>

#include "Sidekick/Core/Window.hpp"

namespace Sidekick::Core
{
class Event;
class WindowCloseEvent;

class Application
{
  public:
    Application();
    ~Application();
    void Run();
    void OnEvent(Event& event);
    bool OnWindowClose(WindowCloseEvent& event);

  private:
    Window m_Window;
    bool m_Running{true};
};

} // namespace Sidekick::Core
