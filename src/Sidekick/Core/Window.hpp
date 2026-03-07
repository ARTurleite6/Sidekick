#pragma once

#include <GLFW/glfw3.h>

#include <string>

namespace Sidekick
{
struct WindowDescriptor
{
  int width;
  int height;
  std::string title;
};

class Window
{
public:
  explicit Window(WindowDescriptor&& descriptor);
  ~Window();

  Window(const Window&) = delete;
  Window& operator=(const Window&) = delete;

  Window(Window&& other) noexcept;
  Window& operator=(Window&& other) noexcept;

  [[nodiscard]] bool IsValid() const;
  [[nodiscard]] bool ShouldClose() const;
  void PollEvents() const;

private:
  static bool AcquireGlfw();
  static void ReleaseGlfw();
  static int s_GlfwWindowOwners;

  void Cleanup();

  WindowDescriptor m_Descriptor;
  GLFWwindow* m_Window{nullptr};

  bool m_OwnsGlfw{false};
};
} // namespace Sidekick
