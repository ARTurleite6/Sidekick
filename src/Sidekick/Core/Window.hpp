#pragma once

#include "Sidekick/Core/Event.hpp"
#include "Sidekick/Renderer/GraphicsContext.hpp"

#include <GLFW/glfw3.h>

#include <functional>
#include <string>

namespace Sidekick
{
struct WindowDescriptor
{
  int Width;
  int Height;
  std::string Title;
  std::function<void(Event&)> EventCallback;
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

  void PollEvents() const;
  [[nodiscard]] GraphicsContext& GetGraphicsContext();
  [[nodiscard]] const GraphicsContext& GetGraphicsContext() const;
  [[nodiscard]] GLFWwindow* GetNativeWindow() const;
  [[nodiscard]] Extent2D GetFramebufferExtent() const;

private:
  static void CloseCallback(GLFWwindow* window);
  static void ResizeCallback(GLFWwindow* window, int width, int height);

  static bool AcquireGlfw();
  static void ReleaseGlfw();
  static int s_GlfwWindowOwners;

  void Cleanup();
  void NotifyEvent(Event& event) const;

  WindowDescriptor m_Descriptor;
  GLFWwindow* m_Window{nullptr};
  GraphicsContext m_GraphicsContext;

  bool m_OwnsGlfw{false};
};
} // namespace Sidekick
