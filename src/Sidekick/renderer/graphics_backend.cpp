#include "sidekick/renderer/graphics_backend.hpp"

#include "sidekick/platform/wgpu/wgpu_backend.hpp"

#include <cassert>
#include <cstdint>
#include <memory>

namespace sidekick
{
std::unique_ptr<graphics_backend> graphics_backend::create()
{
  return std::make_unique<wgpu_graphics_backend>();
}

bool graphics_backend::init(const graphics_context_descriptor& descriptor)
{
  assert(!m_initialized);

  m_initialized = on_init(descriptor);
  return m_initialized;
}

bool graphics_backend::begin_frame()
{
  assert(m_initialized);
  assert(!m_frame_active);
  assert(!m_render_pass_active);

  if (!on_begin_frame())
  {
    return false;
  }

  m_frame_active = true;
  return true;
}

void graphics_backend::begin_render_pass(const render_pass_descriptor& descriptor)
{
  assert(m_initialized);
  assert(m_frame_active);
  assert(!m_render_pass_active);

  on_begin_render_pass(descriptor);
  m_render_pass_active = true;
}

void graphics_backend::end_render_pass()
{
  assert(m_initialized);
  assert(m_frame_active);
  assert(m_render_pass_active);

  on_end_render_pass();
  m_render_pass_active = false;
}

void graphics_backend::end_frame()
{
  assert(m_initialized);
  assert(m_frame_active);
  assert(!m_render_pass_active);

  on_end_frame();
  m_frame_active = false;
}

bool graphics_backend::resize(uint32_t width, uint32_t height)
{
  assert(m_initialized);
  assert(!m_frame_active);
  assert(!m_render_pass_active);

  return on_resize(width, height);
}
} // namespace sidekick
