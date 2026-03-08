#pragma once

#include "sidekick/core/event.hpp"
#include "sidekick/renderer/graphics_backend.hpp"

#include <GLFW/glfw3.h>

#include <functional>
#include <string>

namespace sidekick
{
struct window_descriptor
{
  int width;
  int height;
  std::string title;
  std::function<void(event&)> event_callback;
};

class window
{
public:
  explicit window(window_descriptor&& descriptor);
  ~window();

  window(const window&) = delete;
  window& operator=(const window&) = delete;

  window(window&& other) noexcept;
  window& operator=(window&& other) noexcept;

  void poll_events() const;
  [[nodiscard]] graphics_backend& get_graphics_backend();
  [[nodiscard]] const graphics_backend& get_graphics_backend() const;
  [[nodiscard]] GLFWwindow* get_native_window() const;
  [[nodiscard]] extent_2d get_framebuffer_extent() const;

private:
  static void close_callback(GLFWwindow* window);
  static void resize_callback(GLFWwindow* window, int width, int height);

  static bool acquire_glfw();
  static void release_glfw();
  static int s_glfw_window_owners;

  void cleanup();
  void notify_event(event& event) const;

  window_descriptor m_descriptor;
  GLFWwindow* m_window{nullptr};
  std::unique_ptr<graphics_backend> m_graphics_backend;

  bool m_owns_glfw{false};
};
} // namespace sidekick
