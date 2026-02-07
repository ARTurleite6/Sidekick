#pragma once

#include <GLFW/glfw3.h>

#include <cstdint>
#include <functional>
#include <string>

namespace Sidekick::Core
{
class Event;

struct WindowSpecification
{
    using OnEventCallback = std::function<void(Event&)>;

    std::uint32_t Width, Height;
    std::string Title;
    OnEventCallback OnEvent{nullptr};
};

class Window
{
  public:
    Window(const WindowSpecification& spec = WindowSpecification{});
    Window(const Window&) = delete;
    Window(Window&& other) noexcept;
    Window& operator=(const Window&) = delete;
    Window& operator=(Window&& other) noexcept;
    ~Window() noexcept;

    bool Create();
    void Destroy() noexcept;
    bool ShouldClose() const noexcept;
    void SwapBuffers() noexcept;
    std::pair<int, int> GetSize() const noexcept;
    GLFWwindow* GetHandle() const noexcept;

  private:
    WindowSpecification m_Spec;

    GLFWwindow* m_Window;
};

} // namespace Sidekick::Core
