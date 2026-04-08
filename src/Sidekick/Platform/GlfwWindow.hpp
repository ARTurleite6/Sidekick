#pragma once

#include "Sidekick/Core/Window.hpp"

struct GLFWwindow;

namespace Sidekick
{
class GlfwWindow final : public Window
{
public:
  explicit GlfwWindow(GLFWwindow* window);

  void* GetHandle() const override;

  bool ShouldClose() const override;

private:
  GLFWwindow* m_Window;
};
} // namespace Sidekick
