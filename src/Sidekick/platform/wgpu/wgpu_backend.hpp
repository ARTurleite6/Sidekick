#pragma once

#include "sidekick/renderer/graphics_backend.hpp"

#include "webgpu/webgpu_cpp.h"

#include <cstdint>

namespace sidekick
{
class wgpu_graphics_backend final : public graphics_backend
{
public:
  wgpu_graphics_backend() = default;

private:
  struct frame_context
  {
    wgpu::SurfaceTexture surface_texture{};
    wgpu::CommandEncoder command_encoder{nullptr};
    wgpu::RenderPassEncoder render_pass_encoder{nullptr};

    void reset()
    {
      render_pass_encoder = nullptr;
      command_encoder = nullptr;
      surface_texture = {};
    }
  };

  bool on_init(const graphics_context_descriptor& descriptor) override;
  bool on_begin_frame() override;
  void on_begin_render_pass(const render_pass_descriptor& descriptor) override;
  void on_end_render_pass() override;
  void on_end_frame() override;
  bool on_resize(std::uint32_t width, std::uint32_t height) override;

  bool request_adapter();
  bool request_device();
  bool configure_surface(uint32_t width, uint32_t height);
  bool acquire_surface_texture();

  wgpu::Instance m_instance;
  wgpu::Adapter m_adapter;
  wgpu::Device m_device;
  wgpu::Queue m_queue;
  wgpu::Surface m_surface;
  wgpu::SurfaceConfiguration m_surface_configuration{};

  frame_context m_frame_context{};
};
} // namespace sidekick
