#include "Sidekick/Platform/GlfwWindow.hpp"

#include <GLFW/glfw3.h>

namespace Sidekick
{

GlfwWindow::GlfwWindow(GLFWwindow* window) : m_Window{window} {}

void* GlfwWindow::GetHandle() const
{
  return m_Window;
}

bool GlfwWindow::ShouldClose() const
{
  return glfwWindowShouldClose(m_Window);
}

} // namespace Sidekick
