#include "Sidekick/Core/Window.hpp"

#include <GLFW/glfw3.h>

#include "Sidekick/Core/Event.hpp"
#include "Sidekick/Core/Input.hpp"
#include "Sidekick/Core/KeyCode.hpp"

#include <iostream>

namespace Sidekick::Core
{
Window::Window(const WindowSpecification& spec) : m_Spec{spec}
{
}

Window::Window(Window&& other) noexcept : m_Window{std::exchange(other.m_Window, nullptr)}
{
}

Window& Window::operator=(Window&& other) noexcept
{
    std::swap(m_Window, other.m_Window);
    return *this;
}

Window::~Window() noexcept
{
    Destroy();
}

bool Window::Create()
{
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    m_Window = glfwCreateWindow(640, 480, "Hello World", nullptr, nullptr);
    if (!m_Window)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        return false;
    }

    glfwSetWindowUserPointer(m_Window, this);

    glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* handle) {
        Window& window = *static_cast<Window*>(glfwGetWindowUserPointer(handle));

        if (const auto& OnEvent = window.m_Spec.OnEvent)
        {
            WindowCloseEvent event{};
            OnEvent(event);
        }
    });

    glfwSetKeyCallback(m_Window, [](GLFWwindow* handle, int key, int scancode, int action, int mods) {
        Window& window = *static_cast<Window*>(glfwGetWindowUserPointer(handle));

        if (auto OnEvent = window.m_Spec.OnEvent)
        {
            KeyCode code = GLFWKeyToKeyCode(key);
            if (action == GLFW_PRESS)
            {
                KeyPressedEvent event{code};
                OnEvent(event);
            }
            else if (action == GLFW_RELEASE)
            {
                KeyReleasedEvent event{code};
                OnEvent(event);
            }
        }
    });

    glfwMakeContextCurrent(m_Window);

    return true;
}

void Window::Destroy() noexcept
{
    if (m_Window)
    {
        glfwDestroyWindow(m_Window);
        m_Window = nullptr;
    }
}

bool Window::ShouldClose() const noexcept
{
    return glfwWindowShouldClose(m_Window) == GLFW_TRUE;
}

void Window::SwapBuffers() noexcept
{
    glfwSwapBuffers(m_Window);
}

std::pair<int, int> Window::GetSize() const noexcept
{
    int width, height;
    glfwGetWindowSize(m_Window, &width, &height);

    return std::make_pair(width, height);
}

GLFWwindow* Window::GetHandle() const noexcept
{
    return m_Window;
}

} // namespace Sidekick::Core
