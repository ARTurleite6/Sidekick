#include "Sidekick/Core/Window.hpp"

#include <GLFW/glfw3.h>

#include <iostream>
#include <utility>

namespace Sidekick
{
int Window::s_GlfwWindowOwners{0};

Window::Window(WindowDescriptor&& descriptor) : m_Descriptor{std::move(descriptor)}, m_OwnsGlfw{AcquireGlfw()}
{
  if (!m_OwnsGlfw)
  {
    return;
  }

  m_Window = glfwCreateWindow(m_Descriptor.width, m_Descriptor.height, m_Descriptor.title.c_str(), nullptr, nullptr);
  if (m_Window == nullptr)
  {
    std::cerr << "Failed to create GLFW window\n";
    ReleaseGlfw();
    m_OwnsGlfw = false;
  }
}

Window::~Window()
{
  Cleanup();
}

Window::Window(Window&& other) noexcept
    : m_Descriptor(std::move(other.m_Descriptor)), m_Window(std::exchange(other.m_Window, nullptr)),
      m_OwnsGlfw(std::exchange(other.m_OwnsGlfw, false))
{
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
  m_OwnsGlfw = std::exchange(other.m_OwnsGlfw, false);

  return *this;
}

bool Window::IsValid() const
{
  return m_Window != nullptr;
}

bool Window::ShouldClose() const
{
  return m_Window != nullptr && glfwWindowShouldClose(m_Window) == GLFW_TRUE;
}

void Window::PollEvents() const
{
  if (m_Window != nullptr)
  {
    glfwPollEvents();
  }
}

bool Window::AcquireGlfw()
{
  if (s_GlfwWindowOwners == 0)
  {
    if (glfwInit() == GLFW_FALSE)
    {
      std::cerr << "Failed to initialize GLFW\n";
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

void Window::Cleanup()
{
  if (m_Window != nullptr)
  {
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
