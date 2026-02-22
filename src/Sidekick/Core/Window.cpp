#include "Sidekick/Core/Window.h"

#include "Sidekick/Core/Log.h"

#include <GLFW/glfw3.h>

#include <utility>

namespace Sidekick::Core {

bool Window::s_glfw_initialized = false;
uint32_t Window::s_window_count = 0;

Window::~Window() { Shutdown(); }

Window::Window(Window&& other) noexcept
    : m_native_window(std::exchange(other.m_native_window, nullptr)), m_data(std::move(other.m_data)) {
  if (m_native_window) {
    glfwSetWindowUserPointer(m_native_window, this);
  }
}

Window& Window::operator=(Window&& other) noexcept {
  if (this == &other) {
    return *this;
  }

  Shutdown();

  m_native_window = std::exchange(other.m_native_window, nullptr);
  m_data = std::move(other.m_data);

  if (m_native_window) {
    glfwSetWindowUserPointer(m_native_window, this);
  }

  return *this;
}

bool Window::Initialize(const char* title, uint32_t width, uint32_t height) {
  if (m_native_window) {
    Shutdown();
  }

  if (!s_glfw_initialized) {
    glfwSetErrorCallback(GLFWErrorCallback);
    if (!glfwInit()) {
      return false;
    }
    s_glfw_initialized = true;
  }

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  m_native_window = glfwCreateWindow(static_cast<int>(width), static_cast<int>(height), title, nullptr, nullptr);
  if (!m_native_window) {
    if (s_window_count == 0) {
      glfwTerminate();
      s_glfw_initialized = false;
    }
    return false;
  }

  ++s_window_count;

  m_data.width = width;
  m_data.height = height;

  glfwSetWindowUserPointer(m_native_window, this);
  glfwSetWindowCloseCallback(m_native_window, WindowCloseCallback);
  glfwSetFramebufferSizeCallback(m_native_window, FramebufferSizeCallback);
  glfwSetKeyCallback(m_native_window, KeyCallback);
  glfwSetCursorPosCallback(m_native_window, CursorPosCallback);
  glfwSetMouseButtonCallback(m_native_window, MouseButtonCallback);
  glfwSetScrollCallback(m_native_window, ScrollCallback);

  return true;
}

void Window::Shutdown() {
  if (m_native_window) {
    glfwDestroyWindow(m_native_window);
    m_native_window = nullptr;

    if (s_window_count > 0) {
      --s_window_count;
    }
  }

  if (s_glfw_initialized && s_window_count == 0) {
    glfwTerminate();
    s_glfw_initialized = false;
  }
}

void Window::Update() const { glfwPollEvents(); }

GLFWwindow* Window::GetNativeWindow() const { return m_native_window; }

bool Window::ShouldClose() const {
  if (!m_native_window) {
    return true;
  }
  return glfwWindowShouldClose(m_native_window) != 0;
}

void Window::GetFramebufferSize(uint32_t& width, uint32_t& height) const {
  if (!m_native_window) {
    width = 0;
    height = 0;
    return;
  }

  int framebuffer_width = 0;
  int framebuffer_height = 0;
  glfwGetFramebufferSize(m_native_window, &framebuffer_width, &framebuffer_height);

  width = framebuffer_width > 0 ? static_cast<uint32_t>(framebuffer_width) : 0;
  height = framebuffer_height > 0 ? static_cast<uint32_t>(framebuffer_height) : 0;
}

uint32_t Window::GetWidth() const { return m_data.width; }

uint32_t Window::GetHeight() const { return m_data.height; }

void Window::SetEventCallback(EventCallbackFn callback) { m_data.event_callback = std::move(callback); }

void Window::GLFWErrorCallback(int error_code, const char* description) {
  SK_ERROR("GLFW error ({}): {}", error_code, description);
}

Window* Window::GetWindowFromUserPointer(GLFWwindow* native_window) {
  if (!native_window) {
    return nullptr;
  }
  return static_cast<Window*>(glfwGetWindowUserPointer(native_window));
}

KeyCode Window::MapGLFWKey(int key) {
  switch (key) {
  case GLFW_KEY_W:
    return KeyCode::W;
  case GLFW_KEY_A:
    return KeyCode::A;
  case GLFW_KEY_S:
    return KeyCode::S;
  case GLFW_KEY_D:
    return KeyCode::D;
  case GLFW_KEY_Q:
    return KeyCode::Q;
  case GLFW_KEY_E:
    return KeyCode::E;
  case GLFW_KEY_SPACE:
    return KeyCode::Space;
  case GLFW_KEY_LEFT_SHIFT:
    return KeyCode::LeftShift;
  case GLFW_KEY_RIGHT_SHIFT:
    return KeyCode::RightShift;
  default:
    return KeyCode::Unknown;
  }
}

MouseButton Window::MapGLFWMouseButton(int button) {
  switch (button) {
  case GLFW_MOUSE_BUTTON_RIGHT:
    return MouseButton::Right;
  default:
    return MouseButton::Unknown;
  }
}

void Window::WindowCloseCallback(GLFWwindow* native_window) {
  Window* window = GetWindowFromUserPointer(native_window);
  if (!window) {
    return;
  }
  WindowCloseEvent event;
  window->DispatchEvent(event);
}

void Window::FramebufferSizeCallback(GLFWwindow* native_window, int width, int height) {
  Window* window = GetWindowFromUserPointer(native_window);
  if (!window) {
    return;
  }

  const uint32_t event_width = width > 0 ? static_cast<uint32_t>(width) : 0;
  const uint32_t event_height = height > 0 ? static_cast<uint32_t>(height) : 0;

  if (event_width > 0) {
    window->m_data.width = event_width;
  }
  if (event_height > 0) {
    window->m_data.height = event_height;
  }

  WindowResizeEvent event(event_width, event_height);
  window->DispatchEvent(event);
}

void Window::KeyCallback(GLFWwindow* native_window, int key, int scancode, int action, int mods) {
  (void)scancode;
  (void)mods;

  Window* window = GetWindowFromUserPointer(native_window);
  if (!window) {
    return;
  }

  const KeyCode key_code = MapGLFWKey(key);
  if (action == GLFW_PRESS) {
    KeyPressedEvent event(key_code, 0);
    window->DispatchEvent(event);
  } else if (action == GLFW_REPEAT) {
    KeyPressedEvent event(key_code, 1);
    window->DispatchEvent(event);
  } else if (action == GLFW_RELEASE) {
    KeyReleasedEvent event(key_code);
    window->DispatchEvent(event);
  }
}

void Window::CursorPosCallback(GLFWwindow* native_window, double x_pos, double y_pos) {
  Window* window = GetWindowFromUserPointer(native_window);
  if (!window) {
    return;
  }

  MouseMovedEvent event(x_pos, y_pos);
  window->DispatchEvent(event);
}

void Window::MouseButtonCallback(GLFWwindow* native_window, int button, int action, int mods) {
  (void)mods;

  Window* window = GetWindowFromUserPointer(native_window);
  if (!window) {
    return;
  }

  const MouseButton mapped_button = MapGLFWMouseButton(button);
  if (action == GLFW_PRESS) {
    MouseButtonPressedEvent event(mapped_button);
    window->DispatchEvent(event);
  } else if (action == GLFW_RELEASE) {
    MouseButtonReleasedEvent event(mapped_button);
    window->DispatchEvent(event);
  }
}

void Window::ScrollCallback(GLFWwindow* native_window, double x_offset, double y_offset) {
  Window* window = GetWindowFromUserPointer(native_window);
  if (!window) {
    return;
  }

  MouseScrolledEvent event(x_offset, y_offset);
  window->DispatchEvent(event);
}

void Window::DispatchEvent(Event& event) {
  if (!m_data.event_callback) {
    return;
  }
  m_data.event_callback(event);
}

} // namespace Sidekick::Core
