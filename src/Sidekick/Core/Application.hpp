#pragma once

#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include <print>

namespace Sidekick
{
class Application
{
public:
  Application();
  Application(const Application&) = delete;
  Application(Application&&) = delete;
  Application& operator=(const Application&) = delete;
  Application& operator=(Application&&) = delete;
  ~Application();

  void Run();

private:
  GLFWwindow* m_Window;
  bool m_Running{true};
};
} // namespace Sidekick
