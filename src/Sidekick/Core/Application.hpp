#pragma once

#include "sidekick/core/window.hpp"

namespace sidekick
{
struct event;
struct window_closed_event;
struct window_resized_event;

class application
{
public:
  application();

  void run();

private:
  void on_event(event& event);
  bool on_window_closed(window_closed_event& event);
  bool on_window_resized(window_resized_event& event);

  bool m_running{true};
  bool m_is_minimized{false};
  window m_window;
};
} // namespace sidekick
