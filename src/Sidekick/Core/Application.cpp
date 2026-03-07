#include "Sidekick/Core/Application.hpp"

#include "Sidekick/Core/Event.hpp"
#include "Sidekick/Renderer/GraphicsContext.hpp"

#include <functional>

namespace Sidekick
{
Application::Application()
    : m_Window{{.Width = 1280,
                .Height = 720,
                .Title = "sidekick",
                .EventCallback = [this](Event& event) { OnEvent(event); }}}
{
  const Extent2D framebuffer_extent = m_Window.GetFramebufferExtent();
  m_IsMinimized = framebuffer_extent.Width == 0 || framebuffer_extent.Height == 0;
}

void Application::OnEvent(Event& event)
{
  EventDispatcher dispatcher(&event);
  dispatcher.Dispatch<WindowClosedEvent>(std::bind_front(&Application::OnWindowClosed, this));
  dispatcher.Dispatch<WindowResizeEvent>(std::bind_front(&Application::OnWindowResized, this));
}

bool Application::OnWindowClosed(WindowClosedEvent&)
{
  m_Running = false;
  return true;
}

bool Application::OnWindowResized(WindowResizeEvent& window_resize_event)
{
  if (window_resize_event.Width == 0 || window_resize_event.Height == 0)
  {
    m_IsMinimized = true;
    return true;
  }

  m_IsMinimized = false;
  m_Window.GetGraphicsContext().Resize(static_cast<uint32_t>(window_resize_event.Width),
                                       static_cast<uint32_t>(window_resize_event.Height));
  return true;
}

void Application::Run()
{
  const RenderPassDescriptor render_pass_descriptor{.ColorAttachment = {
                                                        .LoadOperation = LoadOp::Clear,
                                                        .StoreOperation = StoreOp::Store,
                                                        .ClearValue = {.R = 1.0, .G = 0.0, .B = 0.0, .A = 1.0},
                                                    }};

  GraphicsContext& graphics_context = m_Window.GetGraphicsContext();

  while (m_Running)
  {
    m_Window.PollEvents();
    if (!m_Running || m_IsMinimized)
    {
      continue;
    }

    graphics_context.BeginFrame();
    graphics_context.BeginRenderPass(render_pass_descriptor);
    graphics_context.EndRenderPass();
    graphics_context.EndFrame();
  }
}
} // namespace Sidekick
