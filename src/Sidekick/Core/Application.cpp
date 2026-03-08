#include "sidekick/core/application.hpp"

#include "sidekick/core/event.hpp"
#include "sidekick/renderer/graphics_backend.hpp"

#include <cstdint>
#include <functional>

namespace sidekick
{
application::application()
    : m_window{{.width = 1280,
                .height = 720,
                .title = "sidekick",
                .event_callback = [this](event& event) { on_event(event); }}}
{
  const extent_2d framebuffer_extent = m_window.get_framebuffer_extent();
  m_is_minimized = framebuffer_extent.width == 0 || framebuffer_extent.height == 0;
}

void application::on_event(event& event)
{
  event_dispatcher dispatcher(&event);
  dispatcher.dispatch<window_closed_event>(std::bind_front(&application::on_window_closed, this));
  dispatcher.dispatch<window_resized_event>(std::bind_front(&application::on_window_resized, this));
}

bool application::on_window_closed(window_closed_event& event [[maybe_unused]])
{
  m_running = false;
  return true;
}

bool application::on_window_resized(window_resized_event& window_resized_event)
{
  if (window_resized_event.width == 0 || window_resized_event.height == 0)
  {
    m_is_minimized = true;
    return true;
  }

  m_is_minimized = false;
  if (!m_window.get_graphics_backend().resize(static_cast<std::uint32_t>(window_resized_event.width),
                                              static_cast<std::uint32_t>(window_resized_event.height)))
  {
    m_running = false;
    return true;
  }

  return true;
}

void application::run()
{
  const render_pass_descriptor render_pass_descriptor{.color_attachment = {
                                                          .load_operation = load_op::clear,
                                                          .store_operation = store_op::store,
                                                          .clear_value = {.r = 1.0, .g = 0.0, .b = 0.0, .a = 1.0},
                                                      }};

  graphics_backend& graphics_backend = m_window.get_graphics_backend();

  while (m_running)
  {
    m_window.poll_events();
    if (!m_running || m_is_minimized)
    {
      continue;
    }

    if (!graphics_backend.begin_frame())
    {
      continue;
    }

    graphics_backend.begin_render_pass(render_pass_descriptor);
    graphics_backend.end_render_pass();
    graphics_backend.end_frame();
  }
}
} // namespace sidekick
