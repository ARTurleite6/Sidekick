#include "Sidekick/Core/Window.hpp"

#include "Sidekick/Core/Event.hpp"

#include <GLFW/glfw3.h>

#include <cstdint>
#include <stdexcept>
#include <utility>

namespace Sidekick
{
int Window::s_GlfwWindowOwners{0};

Window::Window(WindowDescriptor&& descriptor) : m_Descriptor{std::move(descriptor)}, m_OwnsGlfw{AcquireGlfw()}
{
  if (!m_OwnsGlfw)
  {
    throw std::runtime_error{"Failed to initialize GLFW"};
  }

  try
  {
    m_Window = glfwCreateWindow(m_Descriptor.Width, m_Descriptor.Height, m_Descriptor.Title.c_str(), nullptr, nullptr);
    if (m_Window == nullptr)
    {
      throw std::runtime_error{"Failed to create GLFW window"};
    }

    glfwSetWindowUserPointer(m_Window, this);
    glfwSetWindowCloseCallback(m_Window, CloseCallback);
    glfwSetFramebufferSizeCallback(m_Window, ResizeCallback);

    const Extent2D framebuffer_extent = GetFramebufferExtent();
    const bool initialized =
        m_GraphicsContext.Init({.NativeWindow = m_Window, .FramebufferExtent = framebuffer_extent});
    if (!initialized)
    {
      throw std::runtime_error{"Failed to initialize GraphicsContext"};
    }
  }
  catch (...)
  {
    Cleanup();
    throw;
  }
}

Window::~Window()
{
  Cleanup();
}

Window::Window(Window&& other) noexcept
    : m_Descriptor(std::move(other.m_Descriptor)), m_Window(std::exchange(other.m_Window, nullptr)),
      m_GraphicsContext(std::move(other.m_GraphicsContext)), m_OwnsGlfw(std::exchange(other.m_OwnsGlfw, false))
{
  if (m_Window != nullptr)
  {
    glfwSetWindowUserPointer(m_Window, this);
  }
}

Window& Window::operator=(Window&& other) noexcept
{
  if (this == &other)
  {
    return *this;
  }

  Cleanup();

  m_Descriptor = std::move(other.m_Descriptor);
  m_Window = std::exchange(other.m_Window, nullptr);
  m_GraphicsContext = std::move(other.m_GraphicsContext);
  m_OwnsGlfw = std::exchange(other.m_OwnsGlfw, false);

  if (m_Window != nullptr)
  {
    glfwSetWindowUserPointer(m_Window, this);
  }

  return *this;
}

void Window::PollEvents() const
{
  glfwPollEvents();
}

GraphicsContext& Window::GetGraphicsContext()
{
  return m_GraphicsContext;
}

const GraphicsContext& Window::GetGraphicsContext() const
{
  return m_GraphicsContext;
}

GLFWwindow* Window::GetNativeWindow() const
{
  return m_Window;
}

Extent2D Window::GetFramebufferExtent() const
{
  int width{0};
  int height{0};
  glfwGetFramebufferSize(m_Window, &width, &height);

  return {.Width = static_cast<uint32_t>(width), .Height = static_cast<uint32_t>(height)};
}

void Window::CloseCallback(GLFWwindow* window)
{
  auto* owner = static_cast<Window*>(glfwGetWindowUserPointer(window));
  if (owner == nullptr)
  {
    return;
  }

  Event event{.Kind = WindowClosedEvent{}};
  owner->NotifyEvent(event);
}

void Window::ResizeCallback(GLFWwindow* window, int width, int height)
{
  auto* owner = static_cast<Window*>(glfwGetWindowUserPointer(window));
  if (owner == nullptr)
  {
    return;
  }

  owner->m_Descriptor.Width = width;
  owner->m_Descriptor.Height = height;

  Event event{.Kind = WindowResizeEvent{.Width = width, .Height = height}};
  owner->NotifyEvent(event);
}

bool Window::AcquireGlfw()
{
  if (s_GlfwWindowOwners == 0)
  {
    if (glfwInit() == GLFW_FALSE)
    {
      return false;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  }

  ++s_GlfwWindowOwners;
  return true;
}

void Window::ReleaseGlfw()
{
  if (s_GlfwWindowOwners == 0)
  {
    return;
  }

  --s_GlfwWindowOwners;
  if (s_GlfwWindowOwners == 0)
  {
    glfwTerminate();
  }
}

void Window::NotifyEvent(Event& event) const
{
  if (m_Descriptor.EventCallback)
  {
    m_Descriptor.EventCallback(event);
  }
}

void Window::Cleanup()
{
  m_GraphicsContext = {};

  if (m_Window != nullptr)
  {
    glfwSetWindowUserPointer(m_Window, nullptr);
    glfwDestroyWindow(m_Window);
    m_Window = nullptr;
  }

  if (m_OwnsGlfw)
  {
    ReleaseGlfw();
    m_OwnsGlfw = false;
  }
}
} // namespace Sidekick
