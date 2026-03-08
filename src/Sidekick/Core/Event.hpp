#pragma once

#include <functional>
#include <utility>
#include <variant>

namespace sidekick
{
struct window_closed_event
{
};

struct window_resized_event
{
  int width;
  int height;
};

using event_kind = std::variant<window_closed_event, window_resized_event>;

struct event
{
  bool handled{false};
  event_kind kind;
};

class event_dispatcher
{
public:
  explicit event_dispatcher(event* event) : m_event(event) {}

  template <typename EventType, typename Callback> bool dispatch(Callback&& callback)
  {
    EventType* event = std::get_if<EventType>(&m_event->kind);
    if (event == nullptr)
    {
      return false;
    }

    m_event->handled |= std::invoke(std::forward<Callback>(callback), *event);
    return true;
  }

private:
  event* m_event;
};
} // namespace sidekick
