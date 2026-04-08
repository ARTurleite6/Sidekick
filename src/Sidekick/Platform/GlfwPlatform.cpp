#include "Sidekick/Platform/GlfwPlatform.hpp"

#include "Sidekick/Platform/GlfwWindow.hpp"

#include <glad/gl.h>

#include <GLFW/glfw3.h>

#include <memory>

namespace Sidekick
{

GlfwPlatform::GlfwPlatform()
{
  glfwInit();
}

GlfwPlatform::~GlfwPlatform() noexcept
{
  glfwTerminate();
}

std::unique_ptr<Window> GlfwPlatform::CreateWindow(WindowDescriptor&& desc)
{
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  GLFWwindow* window = glfwCreateWindow(1280, 720, "Sidekick", nullptr, nullptr);
  glfwMakeContextCurrent(window);

  gladLoadGL(glfwGetProcAddress);
  return std::make_unique<GlfwWindow>(window);
}
void GlfwPlatform::Update()
{
  glfwPollEvents();
}

} // namespace Sidekick
