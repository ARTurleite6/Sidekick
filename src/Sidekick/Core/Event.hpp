#pragma once

#include <format>
#include <functional>
#include <string>
#include <string_view>
#include <type_traits>

#include "Sidekick/Core/KeyCode.hpp"

namespace Sidekick::Core
{
enum class EventType
{
    None = 0,
    WindowClose,
    KeyPressed,
    KeyReleased,
};

#define EVENT_CLASS_TYPE(type)                                                                                         \
    static EventType GetStaticType() noexcept                                                                          \
    {                                                                                                                  \
        return EventType::type;                                                                                        \
    }                                                                                                                  \
    EventType GetType() const noexcept override                                                                        \
    {                                                                                                                  \
        return GetStaticType();                                                                                        \
    }                                                                                                                  \
    std::string_view GetName() const override                                                                          \
    {                                                                                                                  \
        return #type;                                                                                                  \
    }

class Event
{
  public:
    bool Handled{false};
    virtual ~Event() = default;
    virtual EventType GetType() const noexcept = 0;
    virtual std::string_view GetName() const = 0;
    virtual std::string ToString() const
    {
        return std::string{GetName()};
    }
};

class KeyEvent : public Event
{
  public:
    KeyCode GetKeyCode() const noexcept
    {
        return m_KeyCode;
    }

  protected:
    KeyEvent(KeyCode code) : m_KeyCode{code}
    {
    }

    KeyCode m_KeyCode;
};

class KeyPressedEvent : public KeyEvent
{
  public:
    EVENT_CLASS_TYPE(KeyPressed)

    KeyPressedEvent(KeyCode code) : KeyEvent{code}
    {
    }

    std::string ToString() const override
    {
        return std::format("KeyPressed {}", m_KeyCode);
    }
};

class KeyReleasedEvent : public KeyEvent
{
  public:
    EVENT_CLASS_TYPE(KeyReleased)

    KeyReleasedEvent(KeyCode code) : KeyEvent{code}
    {
    }

    std::string ToString() const override
    {
        return std::format("KeyReleased {}", m_KeyCode);
    }
};

template <typename T>
concept IsEvent = std::is_base_of_v<Event, T> && requires {
    { T::GetStaticType() } -> std::same_as<EventType>;
};

class WindowCloseEvent : public Event
{
  public:
    EVENT_CLASS_TYPE(WindowClose)
};

class EventDispatcher
{
  public:
    template <typename T> using EventFn = std::function<bool(T&)>;

    EventDispatcher(Event* event) : m_Event{event}
    {
    }

    template <IsEvent T> bool Dispatch(const EventFn<T>& func)
    {
        if (m_Event->GetType() == T::GetStaticType() && !m_Event->Handled)
        {
            m_Event->Handled = func(*static_cast<T*>(m_Event));
            return true;
        }

        return false;
    }

  private:
    Event* m_Event;
};
} // namespace Sidekick::Core
