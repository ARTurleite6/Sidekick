#include "sidekick/core/window.hpp"

#include "sidekick/core/event.hpp"
#include "sidekick/renderer/graphics_backend.hpp"

#include <GLFW/glfw3.h>

#include <cassert>
#include <cstdint>
#include <utility>

namespace sidekick
{
int window::s_glfw_window_owners{0};

window::window(window_descriptor&& descriptor) : m_descriptor{std::move(descriptor)}, m_owns_glfw{acquire_glfw()}
{
  assert(m_owns_glfw);

  m_window = glfwCreateWindow(m_descriptor.width, m_descriptor.height, m_descriptor.title.c_str(), nullptr, nullptr);
  assert(m_window && "Failed to create GLFW window");

  glfwSetWindowUserPointer(m_window, this);
  glfwSetWindowCloseCallback(m_window, close_callback);
  glfwSetFramebufferSizeCallback(m_window, resize_callback);

  const extent_2d framebuffer_extent = get_framebuffer_extent();
  m_graphics_backend = graphics_backend::create();

  assert(m_graphics_backend->init({.native_window = m_window, .framebuffer_extent = framebuffer_extent}) &&
         "Failed to initialize graphics_backend");
}

window::~window()
{
  cleanup();
}

window::window(window&& other) noexcept
    : m_descriptor(std::move(other.m_descriptor)), m_window(std::exchange(other.m_window, nullptr)),
      m_graphics_backend(std::move(other.m_graphics_backend)), m_owns_glfw(std::exchange(other.m_owns_glfw, false))
{
  if (m_window != nullptr)
  {
    glfwSetWindowUserPointer(m_window, this);
  }
}

window& window::operator=(window&& other) noexcept
{
  if (this == &other)
  {
    return *this;
  }

  cleanup();

  m_descriptor = std::move(other.m_descriptor);
  m_window = std::exchange(other.m_window, nullptr);
  m_graphics_backend = std::move(other.m_graphics_backend);
  m_owns_glfw = std::exchange(other.m_owns_glfw, false);

  if (m_window != nullptr)
  {
    glfwSetWindowUserPointer(m_window, this);
  }

  return *this;
}

void window::poll_events() const
{
  glfwPollEvents();
}

graphics_backend& window::get_graphics_backend()
{
  return *m_graphics_backend;
}

const graphics_backend& window::get_graphics_backend() const
{
  return *m_graphics_backend;
}

GLFWwindow* window::get_native_window() const
{
  return m_window;
}

extent_2d window::get_framebuffer_extent() const
{
  int width{0};
  int height{0};
  glfwGetFramebufferSize(m_window, &width, &height);

  return {.width = static_cast<uint32_t>(width), .height = static_cast<uint32_t>(height)};
}

void window::close_callback(GLFWwindow* window)
{
  auto* owner = static_cast<class window*>(glfwGetWindowUserPointer(window));
  if (owner == nullptr)
  {
    return;
  }

  event event{.kind = window_closed_event{}};
  owner->notify_event(event);
}

void window::resize_callback(GLFWwindow* window, int width, int height)
{
  auto* owner = static_cast<class window*>(glfwGetWindowUserPointer(window));
  if (owner == nullptr)
  {
    return;
  }

  owner->m_descriptor.width = width;
  owner->m_descriptor.height = height;

  event event{.kind = window_resized_event{.width = width, .height = height}};
  owner->notify_event(event);
}

bool window::acquire_glfw()
{
  if (s_glfw_window_owners == 0)
  {
    if (glfwInit() == GLFW_FALSE)
    {
      return false;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  }

  ++s_glfw_window_owners;
  return true;
}

void window::release_glfw()
{
  if (s_glfw_window_owners == 0)
  {
    return;
  }

  --s_glfw_window_owners;
  if (s_glfw_window_owners == 0)
  {
    glfwTerminate();
  }
}

void window::notify_event(event& event) const
{
  if (m_descriptor.event_callback)
  {
    m_descriptor.event_callback(event);
  }
}

void window::cleanup()
{
  m_graphics_backend.reset();

  if (m_window != nullptr)
  {
    glfwSetWindowUserPointer(m_window, nullptr);
    glfwDestroyWindow(m_window);
    m_window = nullptr;
  }

  if (m_owns_glfw)
  {
    release_glfw();
    m_owns_glfw = false;
  }
}
} // namespace sidekick
