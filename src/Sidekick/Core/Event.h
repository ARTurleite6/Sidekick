#pragma once

#include "Sidekick/Core/KeyCodes.h"
#include "Sidekick/Core/MouseCodes.h"

#include <cstdint>
#include <string>

namespace Sidekick::Core {

enum class EventType : std::uint8_t {
  None = 0,
  WindowClose,
  WindowResize,
  KeyPressed,
  KeyReleased,
  MouseButtonPressed,
  MouseButtonReleased,
  MouseMoved,
  MouseScrolled,
};

class Event {
public:
  virtual ~Event() = default;
  virtual EventType GetEventType() const = 0;
  virtual const char* GetName() const = 0;
  virtual std::string ToString() const { return GetName(); }

  bool handled = false;
};

class WindowCloseEvent : public Event {
public:
  static EventType GetStaticEventType() { return EventType::WindowClose; }
  EventType GetEventType() const override { return GetStaticEventType(); }
  const char* GetName() const override { return "WindowClose"; }
};

class WindowResizeEvent : public Event {
public:
  WindowResizeEvent(std::uint32_t width, std::uint32_t height) : m_width(width), m_height(height) {}

  static EventType GetStaticEventType() { return EventType::WindowResize; }
  EventType GetEventType() const override { return GetStaticEventType(); }
  const char* GetName() const override { return "WindowResize"; }

  uint32_t GetWidth() const { return m_width; }
  uint32_t GetHeight() const { return m_height; }

private:
  uint32_t m_width;
  uint32_t m_height;
};

class KeyPressedEvent : public Event {
public:
  KeyPressedEvent(KeyCode key_code, int repeat_count) : m_key_code(key_code), m_repeat_count(repeat_count) {}

  static EventType GetStaticEventType() { return EventType::KeyPressed; }
  EventType GetEventType() const override { return GetStaticEventType(); }
  const char* GetName() const override { return "KeyPressed"; }

  KeyCode GetKeyCode() const { return m_key_code; }
  int GetRepeatCount() const { return m_repeat_count; }

private:
  KeyCode m_key_code = KeyCode::Unknown;
  int m_repeat_count;
};

class KeyReleasedEvent : public Event {
public:
  explicit KeyReleasedEvent(KeyCode key_code) : m_key_code(key_code) {}

  static EventType GetStaticEventType() { return EventType::KeyReleased; }
  EventType GetEventType() const override { return GetStaticEventType(); }
  const char* GetName() const override { return "KeyReleased"; }

  KeyCode GetKeyCode() const { return m_key_code; }

private:
  KeyCode m_key_code = KeyCode::Unknown;
};

class MouseButtonPressedEvent : public Event {
public:
  explicit MouseButtonPressedEvent(MouseButton button) : m_button(button) {}

  static EventType GetStaticEventType() { return EventType::MouseButtonPressed; }
  EventType GetEventType() const override { return GetStaticEventType(); }
  const char* GetName() const override { return "MouseButtonPressed"; }

  MouseButton GetButton() const { return m_button; }

private:
  MouseButton m_button = MouseButton::Unknown;
};

class MouseButtonReleasedEvent : public Event {
public:
  explicit MouseButtonReleasedEvent(MouseButton button) : m_button(button) {}

  static EventType GetStaticEventType() { return EventType::MouseButtonReleased; }
  EventType GetEventType() const override { return GetStaticEventType(); }
  const char* GetName() const override { return "MouseButtonReleased"; }

  MouseButton GetButton() const { return m_button; }

private:
  MouseButton m_button = MouseButton::Unknown;
};

class MouseMovedEvent : public Event {
public:
  MouseMovedEvent(double x, double y) : m_x(x), m_y(y) {}

  static EventType GetStaticEventType() { return EventType::MouseMoved; }
  EventType GetEventType() const override { return GetStaticEventType(); }
  const char* GetName() const override { return "MouseMoved"; }

  double GetX() const { return m_x; }
  double GetY() const { return m_y; }

private:
  double m_x;
  double m_y;
};

class MouseScrolledEvent : public Event {
public:
  MouseScrolledEvent(double x_offset, double y_offset) : m_x_offset(x_offset), m_y_offset(y_offset) {}

  static EventType GetStaticEventType() { return EventType::MouseScrolled; }
  EventType GetEventType() const override { return GetStaticEventType(); }
  const char* GetName() const override { return "MouseScrolled"; }

  double GetXOffset() const { return m_x_offset; }
  double GetYOffset() const { return m_y_offset; }

private:
  double m_x_offset;
  double m_y_offset;
};

class EventDispatcher {
public:
  explicit EventDispatcher(Event& event) : m_event(event) {}

  template <typename TEvent, typename THandler> bool Dispatch(const THandler& handler) {
    if (m_event.GetEventType() != TEvent::GetStaticEventType()) {
      return false;
    }

    m_event.handled = m_event.handled || handler(static_cast<TEvent&>(m_event));
    return true;
  }

private:
  Event& m_event;
};

} // namespace Sidekick::Core
