#pragma once

#include <cstdint>
#include <memory>

#include <webgpu/webgpu_cpp.h>

#include "Sidekick/Core/Event.h"
#include "Sidekick/Core/Window.h"
#include "Sidekick/Renderer/Camera.h"
#include "Sidekick/Renderer/CameraController.h"

namespace Sidekick::Core {

class Application {
public:
  Application() = default;
  bool Initialize();
  void Run();
  void Shutdown();

private:
  bool InitializeWindow();
  bool InitializeDawn();
  bool InitializeRenderer();
  void OnEvent(Event& event);
  bool OnWindowClose(WindowCloseEvent& event);
  bool OnWindowResize(WindowResizeEvent& event);
  void ConfigureSurface(uint32_t width, uint32_t height);
  void UpdateSurfaceSize();
  void RenderFrame();

  std::unique_ptr<Window> m_window;
  bool m_running = false;
  uint32_t m_width = 1280;
  uint32_t m_height = 720;

  wgpu::Instance m_instance;
  wgpu::Adapter m_adapter;
  wgpu::Device m_device;
  wgpu::Queue m_queue;
  wgpu::Surface m_surface;
  wgpu::TextureFormat m_surface_format = wgpu::TextureFormat::Undefined;
  wgpu::PresentMode m_present_mode = wgpu::PresentMode::Undefined;
  wgpu::CompositeAlphaMode m_alpha_mode = wgpu::CompositeAlphaMode::Auto;

  wgpu::Texture m_depth_texture;
  wgpu::TextureView m_depth_view;

  wgpu::RenderPipeline m_pipeline;
  wgpu::Buffer m_vertex_buffer;
  wgpu::Buffer m_index_buffer;
  wgpu::Buffer m_uniform_buffer;
  wgpu::BindGroupLayout m_bind_group_layout;
  wgpu::BindGroup m_bind_group;

  float m_last_time = 0.0f;

  std::unique_ptr<Sidekick::Renderer::Camera> m_camera;
  std::unique_ptr<Sidekick::Renderer::CameraController> m_camera_controller;
};

} // namespace Sidekick::Core
