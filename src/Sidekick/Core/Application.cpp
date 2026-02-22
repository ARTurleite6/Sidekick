#include "Sidekick/Core/Application.h"

#include "Sidekick/Core/Input.h"
#include "Sidekick/Core/Log.h"

#include <GLFW/glfw3.h>

#include <dawn/dawn_proc.h>
#include <dawn/native/DawnNative.h>
#include <webgpu/webgpu_glfw.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <array>
#include <cstdint>
#include <limits>
#include <string>

namespace Sidekick::Core {

namespace {

struct Vertex {
  glm::vec3 position;
  glm::vec3 color;
};

constexpr std::array<Vertex, 8> kCubeVertices = {
    Vertex{.position = {-1.0f, -1.0f, -1.0f}, .color = {1.0f, 0.0f, 0.0f}},
    Vertex{.position = {1.0f, -1.0f, -1.0f}, .color = {0.0f, 1.0f, 0.0f}},
    Vertex{.position = {1.0f, 1.0f, -1.0f}, .color = {0.0f, 0.0f, 1.0f}},
    Vertex{.position = {-1.0f, 1.0f, -1.0f}, .color = {1.0f, 1.0f, 0.0f}},
    Vertex{.position = {-1.0f, -1.0f, 1.0f}, .color = {1.0f, 0.0f, 1.0f}},
    Vertex{.position = {1.0f, -1.0f, 1.0f}, .color = {0.0f, 1.0f, 1.0f}},
    Vertex{.position = {1.0f, 1.0f, 1.0f}, .color = {1.0f, 1.0f, 1.0f}},
    Vertex{.position = {-1.0f, 1.0f, 1.0f}, .color = {0.2f, 0.2f, 0.2f}},
};

constexpr std::array<uint16_t, 36> kCubeIndices = {
    0, 3, 2, 2, 1, 0, 4, 5, 6, 6, 7, 4, 0, 4, 7, 7, 3, 0, 1, 2, 6, 6, 5, 1, 3, 7, 6, 6, 2, 3, 0, 1, 5, 5, 4, 0,
};

wgpu::TextureFormat PickSurfaceFormat(const wgpu::SurfaceCapabilities& capabilities) {
  for (size_t index = 0; index < capabilities.formatCount; ++index) {
    const wgpu::TextureFormat format = capabilities.formats[index];
    if (format == wgpu::TextureFormat::BGRA8UnormSrgb || format == wgpu::TextureFormat::RGBA8UnormSrgb) {
      return format;
    }
  }
  if (capabilities.formatCount > 0) {
    return capabilities.formats[0];
  }
  return wgpu::TextureFormat::BGRA8Unorm;
}

wgpu::PresentMode PickPresentMode(const wgpu::SurfaceCapabilities& capabilities) {
  for (size_t index = 0; index < capabilities.presentModeCount; ++index) {
    if (capabilities.presentModes[index] == wgpu::PresentMode::Fifo) {
      return wgpu::PresentMode::Fifo;
    }
  }
  if (capabilities.presentModeCount > 0) {
    return capabilities.presentModes[0];
  }
  return wgpu::PresentMode::Fifo;
}

wgpu::CompositeAlphaMode PickAlphaMode(const wgpu::SurfaceCapabilities& capabilities) {
  for (size_t index = 0; index < capabilities.alphaModeCount; ++index) {
    if (capabilities.alphaModes[index] == wgpu::CompositeAlphaMode::Auto) {
      return wgpu::CompositeAlphaMode::Auto;
    }
  }
  if (capabilities.alphaModeCount > 0) {
    return capabilities.alphaModes[0];
  }
  return wgpu::CompositeAlphaMode::Auto;
}

Sidekick::Renderer::Backend::PixelFormat ToBackendPixelFormat(wgpu::TextureFormat format) {
  switch (format) {
  case wgpu::TextureFormat::BGRA8UnormSrgb:
    return Sidekick::Renderer::Backend::PixelFormat::BGRA8UnormSrgb;
  case wgpu::TextureFormat::RGBA8UnormSrgb:
    return Sidekick::Renderer::Backend::PixelFormat::RGBA8UnormSrgb;
  case wgpu::TextureFormat::Depth24Plus:
    return Sidekick::Renderer::Backend::PixelFormat::Depth24Plus;
  default:
    return Sidekick::Renderer::Backend::PixelFormat::Undefined;
  }
}

} // namespace

bool Application::Initialize() {
  if (!InitializeWindow()) {
    return false;
  }
  Input::Initialize();
  if (!InitializeDawn()) {
    return false;
  }
  if (!InitializeRenderer()) {
    return false;
  }
  m_last_time = static_cast<float>(glfwGetTime());
  return true;
}

void Application::Run() {
  m_running = true;
  while (m_running && m_window && !m_window->ShouldClose()) {
    const auto current_time = static_cast<float>(glfwGetTime());
    const auto delta_time = current_time - m_last_time;
    m_last_time = current_time;

    Input::BeginFrame();
    m_window->Update();
    UpdateSurfaceSize();
    if (m_camera_controller && m_camera) {
      if (m_camera_controller->Update(*m_camera, delta_time)) {
        const glm::mat4 view_proj = m_camera->GetViewProjection();
        m_graphics_backend->UpdateBuffer(m_uniform_buffer, 0, glm::value_ptr(view_proj), sizeof(glm::mat4));
      }
    }
    RenderFrame();
  }
}

void Application::OnEvent(Event& event) {
  Input::OnEvent(event);
  EventDispatcher dispatcher(event);
  dispatcher.Dispatch<WindowCloseEvent>([this](WindowCloseEvent& close_event) { return OnWindowClose(close_event); });
  dispatcher.Dispatch<WindowResizeEvent>(
      [this](WindowResizeEvent& resize_event) { return OnWindowResize(resize_event); });
}

bool Application::OnWindowClose(WindowCloseEvent& event) {
  (void)event;
  m_running = false;
  return true;
}

bool Application::OnWindowResize(WindowResizeEvent& event) {
  if (event.GetWidth() == 0 || event.GetHeight() == 0) {
    return false;
  }

  ConfigureSurface(event.GetWidth(), event.GetHeight());
  if (m_camera) {
    const float aspect = static_cast<float>(event.GetWidth()) / static_cast<float>(event.GetHeight());
    m_camera->SetAspect(aspect);
    const glm::mat4 view_proj = m_camera->GetViewProjection();
    m_graphics_backend->UpdateBuffer(m_uniform_buffer, 0, glm::value_ptr(view_proj), sizeof(glm::mat4));
  }
  return true;
}

void Application::Shutdown() {
  if (m_graphics_backend) {
    m_graphics_backend->Shutdown();
    m_graphics_backend.reset();
  }

  if (m_surface) {
    m_surface.Unconfigure();
  }

  m_queue = nullptr;
  m_device = nullptr;
  m_adapter = nullptr;
  m_surface = nullptr;
  m_instance = nullptr;

  m_camera_controller.reset();
  m_camera.reset();

  m_window.reset();
  Input::Shutdown();
}

bool Application::InitializeWindow() {
  m_window = std::make_unique<Window>();
  if (!m_window->Initialize("Sidekick", m_width, m_height)) {
    m_window.reset();
    return false;
  }

  m_window->SetEventCallback([this](Event& event) { OnEvent(event); });

  return true;
}

bool Application::InitializeDawn() {
  dawnProcSetProcs(&dawn::native::GetProcs());

  static constexpr auto kTimedWaitAny = wgpu::InstanceFeatureName::TimedWaitAny;
  wgpu::InstanceDescriptor instance_desc = {};
  instance_desc.requiredFeatureCount = 1;
  instance_desc.requiredFeatures = &kTimedWaitAny;
  m_instance = wgpu::CreateInstance(&instance_desc);
  if (!m_instance) {
    SK_ERROR("Failed to create WebGPU instance.");
    return false;
  }

  m_surface = wgpu::glfw::CreateSurfaceForWindow(m_instance, m_window->GetNativeWindow());
  if (!m_surface) {
    SK_ERROR("Failed to create WebGPU surface.");
    return false;
  }

  wgpu::RequestAdapterOptions adapter_options = {};
  m_instance.WaitAny(m_instance.RequestAdapter(
                         &adapter_options, wgpu::CallbackMode::WaitAnyOnly,
                         [this](wgpu::RequestAdapterStatus status, wgpu::Adapter adapter, wgpu::StringView message) {
                           if (status != wgpu::RequestAdapterStatus::Success) {
                             std::string message_text(message.data, message.length);
                             SK_ERROR("Failed to get adapter: {}", message_text);
                             return;
                           }
                           m_adapter = std::move(adapter);
                         }),
                     std::numeric_limits<uint64_t>::max());
  if (!m_adapter) {
    return false;
  }

  wgpu::DeviceDescriptor device_desc = {};
  device_desc.SetDeviceLostCallback(
      wgpu::CallbackMode::AllowSpontaneous,
      [this](const wgpu::Device&, wgpu::DeviceLostReason reason, wgpu::StringView message) {
        std::string message_text(message.data, message.length);
        SK_ERROR("Device lost: {} - {}", static_cast<int>(reason), message_text);
        m_running = false;
      });

  m_instance.WaitAny(
      m_adapter.RequestDevice(&device_desc, wgpu::CallbackMode::WaitAnyOnly,
                              [this](wgpu::RequestDeviceStatus status, wgpu::Device device, wgpu::StringView message) {
                                if (status != wgpu::RequestDeviceStatus::Success) {
                                  std::string message_text(message.data, message.length);
                                  SK_ERROR("Failed to get device: {}", message_text);
                                  return;
                                }
                                m_device = std::move(device);
                              }),
      std::numeric_limits<uint64_t>::max());
  if (!m_device) {
    return false;
  }

  m_queue = m_device.GetQueue();

  uint32_t framebuffer_width = 0;
  uint32_t framebuffer_height = 0;
  m_window->GetFramebufferSize(framebuffer_width, framebuffer_height);
  if (framebuffer_width == 0 || framebuffer_height == 0) {
    framebuffer_width = m_width;
    framebuffer_height = m_height;
  }
  ConfigureSurface(framebuffer_width, framebuffer_height);

  return true;
}

bool Application::InitializeRenderer() {
  m_graphics_backend =
      Sidekick::Renderer::Backend::CreateGraphicsBackend(Sidekick::Renderer::Backend::GraphicsBackendType::Wgpu);
  if (!m_graphics_backend) {
    SK_ERROR("Failed to create graphics backend.");
    return false;
  }

  Sidekick::Renderer::Backend::BackendBootstrapContext backend_context = {};
  backend_context.type = Sidekick::Renderer::Backend::GraphicsBackendType::Wgpu;
  backend_context.native_device = &m_device;
  backend_context.native_queue = &m_queue;
  backend_context.color_format = ToBackendPixelFormat(m_surface_format);
  backend_context.depth_format = Sidekick::Renderer::Backend::PixelFormat::Depth24Plus;
  if (!m_graphics_backend->Initialize(backend_context)) {
    SK_ERROR("Failed to initialize graphics backend.");
    return false;
  }

  const float aspect = static_cast<float>(m_width) / static_cast<float>(m_height);
  m_camera = std::make_unique<Sidekick::Renderer::Camera>(45.0f, aspect, 0.1f, 100.0f);
  m_camera->SetPosition(glm::vec3(0.0f, 0.0f, 5.0f));
  m_camera_controller = std::make_unique<Sidekick::Renderer::CameraController>(m_window->GetNativeWindow());

  const char* shader_source = R"(
struct Uniforms {
  view_proj : mat4x4<f32>,
};

@group(0) @binding(0) var<uniform> uniforms : Uniforms;

struct VertexInput {
  @location(0) position : vec3<f32>,
  @location(1) color : vec3<f32>,
};

struct VertexOutput {
  @builtin(position) position : vec4<f32>,
  @location(0) color : vec3<f32>,
};

@vertex
fn vs_main(input : VertexInput) -> VertexOutput {
  var output : VertexOutput;
  output.position = uniforms.view_proj * vec4<f32>(input.position, 1.0);
  output.color = input.color;
  return output;
}

@fragment
fn fs_main(input : VertexOutput) -> @location(0) vec4<f32> {
  return vec4<f32>(input.color, 1.0);
}
)";

  m_vertex_buffer = m_graphics_backend->CreateBuffer({
      .size = sizeof(kCubeVertices),
      .usage = Sidekick::Renderer::Backend::BufferUsageFlags::Vertex |
               Sidekick::Renderer::Backend::BufferUsageFlags::CopyDst,
      .initial_data = kCubeVertices.data(),
      .initial_data_size = sizeof(kCubeVertices),
  });
  m_index_buffer = m_graphics_backend->CreateBuffer({
      .size = sizeof(kCubeIndices),
      .usage =
          Sidekick::Renderer::Backend::BufferUsageFlags::Index | Sidekick::Renderer::Backend::BufferUsageFlags::CopyDst,
      .initial_data = kCubeIndices.data(),
      .initial_data_size = sizeof(kCubeIndices),
  });
  m_uniform_buffer = m_graphics_backend->CreateBuffer({
      .size = sizeof(glm::mat4),
      .usage = Sidekick::Renderer::Backend::BufferUsageFlags::Uniform |
               Sidekick::Renderer::Backend::BufferUsageFlags::CopyDst,
  });

  const auto shader = m_graphics_backend->CreateShader({.source_wgsl = shader_source});

  Sidekick::Renderer::Backend::BindGroupLayoutEntryDesc layout_entry = {};
  layout_entry.binding = 0;
  layout_entry.visibility = Sidekick::Renderer::Backend::ShaderStage::Vertex;
  layout_entry.min_binding_size = sizeof(glm::mat4);
  m_bind_group_layout = m_graphics_backend->CreateBindGroupLayout({
      .entries = &layout_entry,
      .entry_count = 1,
  });

  Sidekick::Renderer::Backend::BindGroupEntryDesc bind_group_entry = {};
  bind_group_entry.binding = 0;
  bind_group_entry.buffer = m_uniform_buffer;
  bind_group_entry.offset = 0;
  bind_group_entry.size = sizeof(glm::mat4);
  m_bind_group = m_graphics_backend->CreateBindGroup({
      .layout = m_bind_group_layout,
      .entries = &bind_group_entry,
      .entry_count = 1,
  });

  std::array<Sidekick::Renderer::Backend::VertexAttributeDesc, 2> attributes = {};
  attributes[0].format = Sidekick::Renderer::Backend::VertexFormat::Float32x3;
  attributes[0].offset = 0;
  attributes[0].shader_location = 0;
  attributes[1].format = Sidekick::Renderer::Backend::VertexFormat::Float32x3;
  attributes[1].offset = sizeof(glm::vec3);
  attributes[1].shader_location = 1;

  Sidekick::Renderer::Backend::VertexBufferLayoutDesc vertex_layout = {};
  vertex_layout.array_stride = sizeof(Vertex);
  vertex_layout.attributes = attributes.data();
  vertex_layout.attribute_count = static_cast<uint32_t>(attributes.size());

  Sidekick::Renderer::Backend::ColorTargetDesc color_target = {};
  color_target.format = ToBackendPixelFormat(m_surface_format);

  Sidekick::Renderer::Backend::BindGroupLayoutHandle bind_group_layouts[] = {m_bind_group_layout};

  Sidekick::Renderer::Backend::PipelineDesc pipeline_desc = {};
  pipeline_desc.shader = shader;
  pipeline_desc.vertex_entry = "vs_main";
  pipeline_desc.fragment_entry = "fs_main";
  pipeline_desc.bind_group_layouts = bind_group_layouts;
  pipeline_desc.bind_group_layout_count = 1;
  pipeline_desc.vertex_buffers = &vertex_layout;
  pipeline_desc.vertex_buffer_count = 1;
  pipeline_desc.color_targets = &color_target;
  pipeline_desc.color_target_count = 1;
  pipeline_desc.has_depth_stencil = true;
  pipeline_desc.depth_stencil = {
      .format = Sidekick::Renderer::Backend::PixelFormat::Depth24Plus,
      .depth_write_enabled = true,
      .depth_compare = Sidekick::Renderer::Backend::CompareFunction::Less,
  };
  pipeline_desc.topology = Sidekick::Renderer::Backend::PrimitiveTopology::TriangleList;
  pipeline_desc.cull_mode = Sidekick::Renderer::Backend::CullMode::Back;
  pipeline_desc.front_face = Sidekick::Renderer::Backend::FrontFace::CCW;

  m_pipeline = m_graphics_backend->CreatePipeline(pipeline_desc);

  if (m_vertex_buffer.id == 0 || m_index_buffer.id == 0 || m_uniform_buffer.id == 0 || m_bind_group_layout.id == 0 ||
      m_bind_group.id == 0 || m_pipeline.id == 0 || shader.id == 0) {
    SK_ERROR("Failed to initialize renderer resources via graphics backend.");
    return false;
  }

  const glm::mat4 view_proj = m_camera->GetViewProjection();
  m_graphics_backend->UpdateBuffer(m_uniform_buffer, 0, glm::value_ptr(view_proj), sizeof(glm::mat4));

  return true;
}

void Application::ConfigureSurface(uint32_t width, uint32_t height) {
  wgpu::SurfaceCapabilities capabilities = {};
  if (!m_surface.GetCapabilities(m_adapter, &capabilities)) {
    SK_ERROR("Failed to query surface capabilities.");
    return;
  }

  m_surface_format = PickSurfaceFormat(capabilities);
  m_present_mode = PickPresentMode(capabilities);
  m_alpha_mode = PickAlphaMode(capabilities);

  wgpu::SurfaceConfiguration config = {};
  config.device = m_device;
  config.format = m_surface_format;
  config.usage = wgpu::TextureUsage::RenderAttachment;
  config.width = width;
  config.height = height;
  config.presentMode = m_present_mode;
  config.alphaMode = m_alpha_mode;
  m_surface.Configure(&config);

  m_width = width;
  m_height = height;

  wgpu::TextureDescriptor depth_desc = {};
  depth_desc.dimension = wgpu::TextureDimension::e2D;
  depth_desc.size = {.width = width, .height = height, .depthOrArrayLayers = 1};
  depth_desc.format = wgpu::TextureFormat::Depth24Plus;
  depth_desc.usage = wgpu::TextureUsage::RenderAttachment;
  m_depth_texture = m_device.CreateTexture(&depth_desc);
  m_depth_view = m_depth_texture.CreateView();
}

void Application::UpdateSurfaceSize() {
  uint32_t framebuffer_width = 0;
  uint32_t framebuffer_height = 0;
  m_window->GetFramebufferSize(framebuffer_width, framebuffer_height);
  if (framebuffer_width == 0 || framebuffer_height == 0) {
    return;
  }

  if (framebuffer_width != m_width || framebuffer_height != m_height) {
    ConfigureSurface(framebuffer_width, framebuffer_height);
    if (m_camera) {
      const float aspect = static_cast<float>(m_width) / static_cast<float>(m_height);
      m_camera->SetAspect(aspect);
      const glm::mat4 view_proj = m_camera->GetViewProjection();
      m_graphics_backend->UpdateBuffer(m_uniform_buffer, 0, glm::value_ptr(view_proj), sizeof(glm::mat4));
    }
  }
}

void Application::RenderFrame() {
  if (!m_surface || !m_device || !m_graphics_backend) {
    return;
  }

  wgpu::SurfaceTexture surface_texture = {};
  m_surface.GetCurrentTexture(&surface_texture);
  if (surface_texture.status != wgpu::SurfaceGetCurrentTextureStatus::SuccessOptimal &&
      surface_texture.status != wgpu::SurfaceGetCurrentTextureStatus::SuccessSuboptimal) {
    if (surface_texture.status == wgpu::SurfaceGetCurrentTextureStatus::Outdated ||
        surface_texture.status == wgpu::SurfaceGetCurrentTextureStatus::Lost ||
        surface_texture.status == wgpu::SurfaceGetCurrentTextureStatus::Timeout) {
      ConfigureSurface(m_width, m_height);
      return;
    }
    SK_ERROR("Failed to acquire surface texture. Status: {}", static_cast<int>(surface_texture.status));
    m_running = false;
    return;
  }

  wgpu::TextureView backbuffer_view = surface_texture.texture.CreateView();

  const Sidekick::Renderer::Backend::FrameBeginDesc frame_begin = {
      .clear_color = {.r = 0.53f, .g = 0.81f, .b = 0.92f, .a = 1.0f},
      .clear_depth = 1.0f,
  };
  const Sidekick::Renderer::Backend::RenderTargetRefs targets = {
      .color_view = &backbuffer_view,
      .depth_view = &m_depth_view,
  };

  if (!m_graphics_backend->BeginFrame(frame_begin, targets)) {
    SK_ERROR("Failed to begin graphics backend frame.");
    m_running = false;
    return;
  }

  m_graphics_backend->SetPipeline(m_pipeline);
  m_graphics_backend->SetBindGroup(0, m_bind_group);
  m_graphics_backend->SetVertexBuffer(0, m_vertex_buffer, 0);
  m_graphics_backend->SetIndexBuffer(m_index_buffer, Sidekick::Renderer::Backend::IndexType::Uint16, 0);
  m_graphics_backend->DrawIndexed(static_cast<uint32_t>(kCubeIndices.size()), 1, 0, 0, 0);
  m_graphics_backend->EndFrameAndSubmit();

  m_surface.Present();
}

} // namespace Sidekick::Core
