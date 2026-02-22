#include "Sidekick/Core/Application.h"

#include <Sidekick/Core/Input.h>

#include <GLFW/glfw3.h>

#include <dawn/dawn_proc.h>
#include <dawn/native/DawnNative.h>
#include <webgpu/webgpu_glfw.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
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
        m_queue.WriteBuffer(m_uniform_buffer, 0, glm::value_ptr(view_proj), sizeof(glm::mat4));
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
    m_queue.WriteBuffer(m_uniform_buffer, 0, glm::value_ptr(view_proj), sizeof(glm::mat4));
  }
  return true;
}

void Application::Shutdown() {
  if (m_surface) {
    m_surface.Unconfigure();
  }
  m_queue = nullptr;
  m_device = nullptr;
  m_adapter = nullptr;
  m_surface = nullptr;
  m_instance = nullptr;

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
    std::cerr << "Failed to create WebGPU instance.\n";
    return false;
  }

  m_surface = wgpu::glfw::CreateSurfaceForWindow(m_instance, m_window->GetNativeWindow());
  if (!m_surface) {
    std::cerr << "Failed to create WebGPU surface.\n";
    return false;
  }

  wgpu::RequestAdapterOptions adapter_options = {};
  m_instance.WaitAny(m_instance.RequestAdapter(
                         &adapter_options, wgpu::CallbackMode::WaitAnyOnly,
                         [this](wgpu::RequestAdapterStatus status, wgpu::Adapter adapter, wgpu::StringView message) {
                           if (status != wgpu::RequestAdapterStatus::Success) {
                             std::string message_text(message.data, message.length);
                             std::cerr << "Failed to get adapter: " << message_text << '\n';
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
        std::cerr << "Device lost: " << static_cast<int>(reason) << " - " << message_text << '\n';
        m_running = false;
      });

  m_instance.WaitAny(
      m_adapter.RequestDevice(&device_desc, wgpu::CallbackMode::WaitAnyOnly,
                              [this](wgpu::RequestDeviceStatus status, wgpu::Device device, wgpu::StringView message) {
                                if (status != wgpu::RequestDeviceStatus::Success) {
                                  std::string message_text(message.data, message.length);
                                  std::cerr << "Failed to get device: " << message_text << '\n';
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

  wgpu::ShaderSourceWGSL wgsl_desc = {};
  wgsl_desc.code = shader_source;

  wgpu::ShaderModuleDescriptor shader_desc = {};
  shader_desc.nextInChain = &wgsl_desc;
  wgpu::ShaderModule shader_module = m_device.CreateShaderModule(&shader_desc);

  wgpu::BindGroupLayoutEntry uniform_layout = {};
  uniform_layout.binding = 0;
  uniform_layout.visibility = wgpu::ShaderStage::Vertex;
  uniform_layout.buffer.type = wgpu::BufferBindingType::Uniform;
  uniform_layout.buffer.minBindingSize = sizeof(glm::mat4);

  wgpu::BindGroupLayoutDescriptor bind_group_layout_desc = {};
  bind_group_layout_desc.entryCount = 1;
  bind_group_layout_desc.entries = &uniform_layout;
  m_bind_group_layout = m_device.CreateBindGroupLayout(&bind_group_layout_desc);

  wgpu::PipelineLayoutDescriptor pipeline_layout_desc = {};
  pipeline_layout_desc.bindGroupLayoutCount = 1;
  pipeline_layout_desc.bindGroupLayouts = &m_bind_group_layout;
  wgpu::PipelineLayout pipeline_layout = m_device.CreatePipelineLayout(&pipeline_layout_desc);

  wgpu::BufferDescriptor vertex_desc = {};
  vertex_desc.size = sizeof(kCubeVertices);
  vertex_desc.usage = wgpu::BufferUsage::Vertex | wgpu::BufferUsage::CopyDst;
  m_vertex_buffer = m_device.CreateBuffer(&vertex_desc);
  m_queue.WriteBuffer(m_vertex_buffer, 0, kCubeVertices.data(), sizeof(kCubeVertices));

  wgpu::BufferDescriptor index_desc = {};
  index_desc.size = sizeof(kCubeIndices);
  index_desc.usage = wgpu::BufferUsage::Index | wgpu::BufferUsage::CopyDst;
  m_index_buffer = m_device.CreateBuffer(&index_desc);
  m_queue.WriteBuffer(m_index_buffer, 0, kCubeIndices.data(), sizeof(kCubeIndices));

  wgpu::BufferDescriptor uniform_desc = {};
  uniform_desc.size = sizeof(glm::mat4);
  uniform_desc.usage = wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst;
  m_uniform_buffer = m_device.CreateBuffer(&uniform_desc);

  wgpu::BindGroupEntry bind_group_entry = {};
  bind_group_entry.binding = 0;
  bind_group_entry.buffer = m_uniform_buffer;
  bind_group_entry.offset = 0;
  bind_group_entry.size = sizeof(glm::mat4);

  wgpu::BindGroupDescriptor bind_group_desc = {};
  bind_group_desc.layout = m_bind_group_layout;
  bind_group_desc.entryCount = 1;
  bind_group_desc.entries = &bind_group_entry;
  m_bind_group = m_device.CreateBindGroup(&bind_group_desc);

  std::array<wgpu::VertexAttribute, 2> attributes = {};
  attributes[0].format = wgpu::VertexFormat::Float32x3;
  attributes[0].offset = 0;
  attributes[0].shaderLocation = 0;

  attributes[1].format = wgpu::VertexFormat::Float32x3;
  attributes[1].offset = sizeof(glm::vec3);
  attributes[1].shaderLocation = 1;

  wgpu::VertexBufferLayout vertex_layout = {};
  vertex_layout.arrayStride = sizeof(Vertex);
  vertex_layout.stepMode = wgpu::VertexStepMode::Vertex;
  vertex_layout.attributeCount = 2;
  vertex_layout.attributes = attributes.data();

  wgpu::ColorTargetState color_target = {};
  color_target.format = m_surface_format;

  wgpu::FragmentState fragment_state = {};
  fragment_state.module = shader_module;
  fragment_state.entryPoint = "fs_main";
  fragment_state.targetCount = 1;
  fragment_state.targets = &color_target;

  wgpu::DepthStencilState depth_state = {};
  depth_state.format = wgpu::TextureFormat::Depth24Plus;
  depth_state.depthWriteEnabled = true;
  depth_state.depthCompare = wgpu::CompareFunction::Less;

  wgpu::RenderPipelineDescriptor pipeline_desc = {};
  pipeline_desc.layout = pipeline_layout;
  pipeline_desc.vertex.module = shader_module;
  pipeline_desc.vertex.entryPoint = "vs_main";
  pipeline_desc.vertex.bufferCount = 1;
  pipeline_desc.vertex.buffers = &vertex_layout;
  pipeline_desc.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
  pipeline_desc.primitive.cullMode = wgpu::CullMode::Back;
  pipeline_desc.primitive.frontFace = wgpu::FrontFace::CCW;
  pipeline_desc.fragment = &fragment_state;
  pipeline_desc.depthStencil = &depth_state;
  pipeline_desc.multisample.count = 1;
  m_pipeline = m_device.CreateRenderPipeline(&pipeline_desc);

  const glm::mat4 view_proj = m_camera->GetViewProjection();
  m_queue.WriteBuffer(m_uniform_buffer, 0, glm::value_ptr(view_proj), sizeof(glm::mat4));

  ConfigureSurface(m_width, m_height);
  return true;
}

void Application::ConfigureSurface(uint32_t width, uint32_t height) {
  wgpu::SurfaceCapabilities capabilities = {};
  if (!m_surface.GetCapabilities(m_adapter, &capabilities)) {
    std::cerr << "Failed to query surface capabilities.\n";
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
      m_queue.WriteBuffer(m_uniform_buffer, 0, glm::value_ptr(view_proj), sizeof(glm::mat4));
    }
  }
}

void Application::RenderFrame() {
  if (!m_surface || !m_device) {
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
    std::cerr << "Failed to acquire surface texture. Status: " << static_cast<int>(surface_texture.status) << '\n';
    m_running = false;
    return;
  }

  wgpu::TextureView backbuffer_view = surface_texture.texture.CreateView();

  wgpu::CommandEncoder encoder = m_device.CreateCommandEncoder();

  wgpu::RenderPassColorAttachment color_attachment = {};
  color_attachment.view = backbuffer_view;
  color_attachment.loadOp = wgpu::LoadOp::Clear;
  color_attachment.storeOp = wgpu::StoreOp::Store;
  color_attachment.clearValue = {.r = 0.53f, .g = 0.81f, .b = 0.92f, .a = 1.0f};

  wgpu::RenderPassDepthStencilAttachment depth_attachment = {};
  depth_attachment.view = m_depth_view;
  depth_attachment.depthLoadOp = wgpu::LoadOp::Clear;
  depth_attachment.depthStoreOp = wgpu::StoreOp::Store;
  depth_attachment.depthClearValue = 1.0f;

  wgpu::RenderPassDescriptor render_pass_desc = {};
  render_pass_desc.colorAttachmentCount = 1;
  render_pass_desc.colorAttachments = &color_attachment;
  render_pass_desc.depthStencilAttachment = &depth_attachment;

  wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&render_pass_desc);
  pass.SetPipeline(m_pipeline);
  pass.SetBindGroup(0, m_bind_group);
  pass.SetVertexBuffer(0, m_vertex_buffer);
  pass.SetIndexBuffer(m_index_buffer, wgpu::IndexFormat::Uint16);
  pass.DrawIndexed(static_cast<uint32_t>(sizeof(kCubeIndices) / sizeof(kCubeIndices[0])));
  pass.End();

  wgpu::CommandBuffer commands = encoder.Finish();
  m_queue.Submit(1, &commands);
  m_surface.Present();
}

} // namespace Sidekick::Core
