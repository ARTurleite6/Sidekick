#include "Sidekick/Core/Input.h"

#include <array>
#include <cstddef>

namespace Sidekick::Core {

namespace {

constexpr size_t kKeyCount = static_cast<size_t>(KeyCode::Count);
constexpr size_t kMouseButtonCount = static_cast<size_t>(MouseButton::Count);

std::array<uint8_t, kKeyCount> s_key_states{};
std::array<uint8_t, kMouseButtonCount> s_mouse_button_states{};

double s_mouse_x = 0.0;
double s_mouse_y = 0.0;
double s_mouse_delta_x = 0.0;
double s_mouse_delta_y = 0.0;
bool s_has_mouse_position = false;

size_t ToIndex(KeyCode key_code) {
  return static_cast<size_t>(key_code);
}

size_t ToIndex(MouseButton button) {
  return static_cast<size_t>(button);
}

}  // namespace

void Input::Initialize() {
  s_key_states.fill(0);
  s_mouse_button_states.fill(0);
  s_mouse_x = 0.0;
  s_mouse_y = 0.0;
  s_mouse_delta_x = 0.0;
  s_mouse_delta_y = 0.0;
  s_has_mouse_position = false;
}

void Input::Shutdown() {
  s_key_states.fill(0);
  s_mouse_button_states.fill(0);
  s_mouse_x = 0.0;
  s_mouse_y = 0.0;
  s_mouse_delta_x = 0.0;
  s_mouse_delta_y = 0.0;
  s_has_mouse_position = false;
}

void Input::BeginFrame() {
  s_mouse_delta_x = 0.0;
  s_mouse_delta_y = 0.0;
}

void Input::OnEvent(Event& event) {
  EventDispatcher dispatcher(event);
  dispatcher.Dispatch<KeyPressedEvent>([](KeyPressedEvent& key_pressed_event) {
    const KeyCode key_code = key_pressed_event.GetKeyCode();
    if (key_code == KeyCode::Unknown) {
      return false;
    }
    const size_t index = ToIndex(key_code);
    if (index >= s_key_states.size()) {
      return false;
    }
    s_key_states[index] = 1;
    return false;
  });
  dispatcher.Dispatch<KeyReleasedEvent>([](KeyReleasedEvent& key_released_event) {
    const KeyCode key_code = key_released_event.GetKeyCode();
    if (key_code == KeyCode::Unknown) {
      return false;
    }
    const size_t index = ToIndex(key_code);
    if (index >= s_key_states.size()) {
      return false;
    }
    s_key_states[index] = 0;
    return false;
  });
  dispatcher.Dispatch<MouseButtonPressedEvent>([](MouseButtonPressedEvent& button_pressed_event) {
    const MouseButton button = button_pressed_event.GetButton();
    if (button == MouseButton::Unknown) {
      return false;
    }
    const size_t index = ToIndex(button);
    if (index >= s_mouse_button_states.size()) {
      return false;
    }
    s_mouse_button_states[index] = 1;
    return false;
  });
  dispatcher.Dispatch<MouseButtonReleasedEvent>([](MouseButtonReleasedEvent& button_released_event) {
    const MouseButton button = button_released_event.GetButton();
    if (button == MouseButton::Unknown) {
      return false;
    }
    const size_t index = ToIndex(button);
    if (index >= s_mouse_button_states.size()) {
      return false;
    }
    s_mouse_button_states[index] = 0;
    return false;
  });
  dispatcher.Dispatch<MouseMovedEvent>([](MouseMovedEvent& mouse_moved_event) {
    const double next_x = mouse_moved_event.GetX();
    const double next_y = mouse_moved_event.GetY();
    if (s_has_mouse_position) {
      s_mouse_delta_x += next_x - s_mouse_x;
      s_mouse_delta_y += next_y - s_mouse_y;
    } else {
      s_has_mouse_position = true;
    }
    s_mouse_x = next_x;
    s_mouse_y = next_y;
    return false;
  });
}

bool Input::IsKeyPressed(KeyCode key_code) {
  if (key_code == KeyCode::Unknown) {
    return false;
  }
  const size_t index = ToIndex(key_code);
  return index < s_key_states.size() && s_key_states[index] != 0;
}

bool Input::IsMouseButtonPressed(MouseButton button) {
  if (button == MouseButton::Unknown) {
    return false;
  }
  const size_t index = ToIndex(button);
  return index < s_mouse_button_states.size() && s_mouse_button_states[index] != 0;
}

double Input::GetMouseX() {
  return s_mouse_x;
}

double Input::GetMouseY() {
  return s_mouse_y;
}

double Input::GetMouseDeltaX() {
  return s_mouse_delta_x;
}

double Input::GetMouseDeltaY() {
  return s_mouse_delta_y;
}

}  // namespace Sidekick::Core
