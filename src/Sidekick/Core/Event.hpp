#pragma once

#include <functional>
#include <utility>
#include <variant>

namespace Sidekick
{
struct WindowClosedEvent
{
};

struct WindowResizeEvent
{
  int Width;
  int Height;
};

using EventKind = std::variant<WindowClosedEvent, WindowResizeEvent>;

struct Event
{
  bool Handled{false};
  EventKind Kind;
};

class EventDispatcher
{
public:
  explicit EventDispatcher(Event* event) : m_Event(event) {}

  template <typename EventType, typename Callback> bool Dispatch(Callback&& callback)
  {
    EventType* event = std::get_if<EventType>(&m_Event->Kind);
    if (event == nullptr)
    {
      return false;
    }

    m_Event->Handled |= std::invoke(std::forward<Callback>(callback), *event);
    return true;
  }

private:
  Event* m_Event;
};
} // namespace Sidekick
