#pragma once

#include "Sidekick/Core/Event.h"
#include "Sidekick/Core/KeyCodes.h"
#include "Sidekick/Core/MouseCodes.h"

#include <cstdint>
#include <functional>

struct GLFWwindow;

namespace Sidekick::Core
{

class Window
{
public:
  using EventCallbackFn = std::function<void(Event&)>;

  Window() = default;
  ~Window();

  Window(const Window&) = delete;
  Window& operator=(const Window&) = delete;

  Window(Window&& other) noexcept;
  Window& operator=(Window&& other) noexcept;

  bool Initialize(const char* title, uint32_t width, uint32_t height);
  void Shutdown();

  void Update() const;

  GLFWwindow* GetNativeWindow() const;
  bool ShouldClose() const;
  void GetFramebufferSize(uint32_t& width, uint32_t& height) const;

  uint32_t GetWidth() const;
  uint32_t GetHeight() const;

  void SetEventCallback(EventCallbackFn callback);

private:
  struct WindowData
  {
    uint32_t width = 0;
    uint32_t height = 0;
    EventCallbackFn event_callback;
  };

  static void GLFWErrorCallback(int error_code, const char* description);
  static Window* GetWindowFromUserPointer(GLFWwindow* native_window);
  static KeyCode MapGLFWKey(int key);
  static MouseButton MapGLFWMouseButton(int button);

  static void WindowCloseCallback(GLFWwindow* native_window);
  static void FramebufferSizeCallback(GLFWwindow* native_window, int width, int height);
  static void KeyCallback(GLFWwindow* native_window, int key, int scancode, int action, int mods);
  static void CursorPosCallback(GLFWwindow* native_window, double x_pos, double y_pos);
  static void MouseButtonCallback(GLFWwindow* native_window, int button, int action, int mods);
  static void ScrollCallback(GLFWwindow* native_window, double x_offset, double y_offset);

  void DispatchEvent(Event& event);

  GLFWwindow* m_native_window = nullptr;
  WindowData m_data;

  static bool s_glfw_initialized;
  static uint32_t s_window_count;
};

} // namespace Sidekick::Core
