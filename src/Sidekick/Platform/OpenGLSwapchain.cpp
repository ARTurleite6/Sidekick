#include "Sidekick/Platform/OpenGLSwapchain.hpp"

#include "Sidekick/Core/Swapchain.hpp"

#include <glad/gl.h>

#include <GLFW/glfw3.h>

namespace Sidekick
{

OpenGLSwapchain::OpenGLSwapchain(SwapchainDescriptor&& desc) : m_Window{static_cast<GLFWwindow*>(desc.window_handle)}
{
  glViewport(0, 0, desc.size_x, desc.size_y);
}

void OpenGLSwapchain::Present()
{
  glfwSwapBuffers(m_Window);
}

} // namespace Sidekick
