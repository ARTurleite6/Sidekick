#include "Sidekick/Renderer/GraphicsContext.hpp"

#include <webgpu/webgpu_cpp.h>
#include <webgpu/webgpu_cpp_print.h>
#include <webgpu/webgpu_glfw.h>

#include <cassert>
#include <cstdint>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

namespace Sidekick
{
namespace
{
wgpu::LoadOp ToWgpu(LoadOp load_operation)
{
  switch (load_operation)
  {
  case LoadOp::Load:
    return wgpu::LoadOp::Load;
  case LoadOp::Clear:
    return wgpu::LoadOp::Clear;
  }

  return wgpu::LoadOp::Undefined;
}

wgpu::StoreOp ToWgpu(StoreOp store_operation)
{
  switch (store_operation)
  {
  case StoreOp::Discard:
    return wgpu::StoreOp::Discard;
  case StoreOp::Store:
    return wgpu::StoreOp::Store;
  }

  return wgpu::StoreOp::Undefined;
}

wgpu::Color ToWgpu(const ClearColor& clear_color)
{
  return {.r = clear_color.R, .g = clear_color.G, .b = clear_color.B, .a = clear_color.A};
}

struct AdapterRequestState
{
  wgpu::Adapter Adapter;
  std::string Message;
};

struct DeviceRequestState
{
  wgpu::Device Device;
  std::string Message;
};

void AdapterRequestCallback(wgpu::RequestAdapterStatus status, wgpu::Adapter adapter, wgpu::StringView message,
                            AdapterRequestState* state)
{
  if (status != wgpu::RequestAdapterStatus::Success)
  {
    std::ostringstream stream;
    stream << "status=" << status;
    if (message.data != nullptr)
    {
      stream << " message=" << message;
    }
    state->Message = stream.str();
    return;
  }

  state->Adapter = std::move(adapter);
}

void DeviceRequestCallback(wgpu::RequestDeviceStatus status, wgpu::Device device, wgpu::StringView message,
                           DeviceRequestState* state)
{
  if (status != wgpu::RequestDeviceStatus::Success)
  {
    std::ostringstream stream;
    stream << "status=" << status;
    if (message.data != nullptr)
    {
      stream << " message=" << message;
    }
    state->Message = stream.str();
    return;
  }

  state->Device = std::move(device);
}

void UncapturedErrorCallback(const wgpu::Device& device [[maybe_unused]], wgpu::ErrorType error_type,
                             wgpu::StringView message)
{
  std::cerr << "Uncaptured WebGPU error (" << error_type << "): " << message << '\n';
}

void DeviceLostCallback(const wgpu::Device& device [[maybe_unused]], wgpu::DeviceLostReason reason,
                        wgpu::StringView message)
{
  std::cerr << "WebGPU device lost (" << reason << "): " << message << '\n';
}
} // namespace

class GraphicsBackend
{
public:
  GraphicsBackend() = default;
  GraphicsBackend(const GraphicsBackend&) = default;
  GraphicsBackend(GraphicsBackend&&) noexcept = default;
  GraphicsBackend& operator=(const GraphicsBackend&) = default;
  GraphicsBackend& operator=(GraphicsBackend&&) noexcept = default;
  virtual ~GraphicsBackend() = default;

  virtual bool Init(const GraphicsContextDescriptor& descriptor) = 0;

  virtual void BeginFrame() = 0;
  virtual void BeginRenderPass(const RenderPassDescriptor& descriptor) = 0;
  virtual void EndRenderPass() = 0;
  virtual void EndFrame() = 0;
  virtual void Resize(uint32_t width, uint32_t height) = 0;
};

class WgpuGraphicsBackend final : public GraphicsBackend
{
public:
  bool Init(const GraphicsContextDescriptor& descriptor) override;

  void BeginFrame() override;
  void BeginRenderPass(const RenderPassDescriptor& descriptor) override;
  void EndRenderPass() override;
  void EndFrame() override;
  void Resize(uint32_t width, uint32_t height) override;

private:
  bool RequestAdapter();
  bool RequestDevice();
  bool ConfigureSurface(uint32_t width, uint32_t height);
  bool AcquireSurfaceTexture();
  void ResetTransientState();

  wgpu::Instance m_Instance;
  wgpu::Adapter m_Adapter;
  wgpu::Device m_Device;
  wgpu::Queue m_Queue;
  wgpu::Surface m_Surface;
  wgpu::SurfaceConfiguration m_SurfaceConfiguration{};

  wgpu::SurfaceTexture m_SurfaceTexture{};
  wgpu::CommandEncoder m_CommandEncoder;
  wgpu::RenderPassEncoder m_RenderPassEncoder;
};

GraphicsContext::~GraphicsContext() = default;

GraphicsContext::GraphicsContext() = default;

GraphicsContext::GraphicsContext(GraphicsContext&& other) noexcept = default;

GraphicsContext& GraphicsContext::operator=(GraphicsContext&& other) noexcept = default;

bool GraphicsContext::Init(const GraphicsContextDescriptor& descriptor)
{
  assert(m_Backend == nullptr);
  if (m_Backend != nullptr)
  {
    return false;
  }

  auto backend = std::make_unique<WgpuGraphicsBackend>();
  if (!backend->Init(descriptor))
  {
    return false;
  }

  m_Backend = std::move(backend);
  return true;
}

void GraphicsContext::BeginFrame()
{
  assert(m_Backend != nullptr);
  assert(!m_FrameActive);
  assert(!m_RenderPassActive);

  m_Backend->BeginFrame();
  m_FrameActive = true;
}

void GraphicsContext::BeginRenderPass(const RenderPassDescriptor& descriptor)
{
  assert(m_Backend != nullptr);
  assert(m_FrameActive);
  assert(!m_RenderPassActive);

  m_Backend->BeginRenderPass(descriptor);
  m_RenderPassActive = true;
}

void GraphicsContext::EndRenderPass()
{
  assert(m_Backend != nullptr);
  assert(m_FrameActive);
  assert(m_RenderPassActive);

  m_Backend->EndRenderPass();
  m_RenderPassActive = false;
}

void GraphicsContext::EndFrame()
{
  assert(m_Backend != nullptr);
  assert(m_FrameActive);
  assert(!m_RenderPassActive);

  m_Backend->EndFrame();
  m_FrameActive = false;
}

void GraphicsContext::Resize(uint32_t width, uint32_t height)
{
  assert(m_Backend != nullptr);
  assert(!m_FrameActive);
  assert(!m_RenderPassActive);

  m_Backend->Resize(width, height);
}

bool WgpuGraphicsBackend::Init(const GraphicsContextDescriptor& descriptor)
{
  if (descriptor.NativeWindow == nullptr)
  {
    std::cerr << "Failed to initialize WebGPU backend: missing native window\n";
    return false;
  }

  static constexpr auto kTimedWaitAny = wgpu::InstanceFeatureName::TimedWaitAny;
  wgpu::InstanceDescriptor instance_descriptor{};
  instance_descriptor.requiredFeatureCount = 1;
  instance_descriptor.requiredFeatures = &kTimedWaitAny;

  m_Instance = wgpu::CreateInstance(&instance_descriptor);
  if (m_Instance == nullptr)
  {
    std::cerr << "Failed to create WebGPU instance\n";
    return false;
  }

  m_Surface = wgpu::glfw::CreateSurfaceForWindow(m_Instance, descriptor.NativeWindow);
  if (m_Surface == nullptr)
  {
    std::cerr << "Failed to create WebGPU surface for GLFW window\n";
    return false;
  }

  if (!RequestAdapter())
  {
    return false;
  }

  if (!RequestDevice())
  {
    return false;
  }

  return ConfigureSurface(descriptor.FramebufferExtent.Width, descriptor.FramebufferExtent.Height);
}

void WgpuGraphicsBackend::BeginFrame()
{
  assert(m_CommandEncoder == nullptr);
  assert(m_RenderPassEncoder == nullptr);

  ResetTransientState();

  if (!AcquireSurfaceTexture())
  {
    throw std::runtime_error{"Failed to acquire WebGPU surface texture"};
  }

  m_CommandEncoder = m_Device.CreateCommandEncoder();
  if (m_CommandEncoder == nullptr)
  {
    ResetTransientState();
    throw std::runtime_error{"Failed to create WebGPU command encoder"};
  }
}

void WgpuGraphicsBackend::BeginRenderPass(const RenderPassDescriptor& descriptor)
{
  assert(m_CommandEncoder != nullptr);
  assert(m_RenderPassEncoder == nullptr);
  assert(m_SurfaceTexture.texture != nullptr);

  const wgpu::TextureView texture_view = m_SurfaceTexture.texture.CreateView();
  if (texture_view == nullptr)
  {
    throw std::runtime_error{"Failed to create WebGPU texture view"};
  }

  wgpu::RenderPassColorAttachment color_attachment{};
  color_attachment.view = texture_view;
  color_attachment.loadOp = ToWgpu(descriptor.ColorAttachment.LoadOperation);
  color_attachment.storeOp = ToWgpu(descriptor.ColorAttachment.StoreOperation);
  color_attachment.clearValue = ToWgpu(descriptor.ColorAttachment.ClearValue);

  wgpu::RenderPassDescriptor render_pass_descriptor{};
  render_pass_descriptor.colorAttachmentCount = 1;
  render_pass_descriptor.colorAttachments = &color_attachment;

  m_RenderPassEncoder = m_CommandEncoder.BeginRenderPass(&render_pass_descriptor);
  if (m_RenderPassEncoder == nullptr)
  {
    throw std::runtime_error{"Failed to begin WebGPU render pass"};
  }
}

void WgpuGraphicsBackend::EndRenderPass()
{
  assert(m_RenderPassEncoder != nullptr);

  m_RenderPassEncoder.End();
  m_RenderPassEncoder = nullptr;
}

void WgpuGraphicsBackend::EndFrame()
{
  assert(m_CommandEncoder != nullptr);
  assert(m_RenderPassEncoder == nullptr);

  const wgpu::CommandBuffer command_buffer = m_CommandEncoder.Finish();
  if (command_buffer == nullptr)
  {
    ResetTransientState();
    throw std::runtime_error{"Failed to finish WebGPU command buffer"};
  }

  m_CommandEncoder = nullptr;
  m_Queue.Submit(1, &command_buffer);

  const wgpu::Status present_status = m_Surface.Present();
  ResetTransientState();

  if (present_status != wgpu::Status::Success)
  {
    throw std::runtime_error{"Failed to present WebGPU surface"};
  }
}

void WgpuGraphicsBackend::Resize(uint32_t width, uint32_t height)
{
  assert(m_CommandEncoder == nullptr);
  assert(m_RenderPassEncoder == nullptr);

  if (!ConfigureSurface(width, height))
  {
    throw std::runtime_error{"Failed to resize WebGPU surface"};
  }
}

bool WgpuGraphicsBackend::RequestAdapter()
{
  AdapterRequestState request_state{};

  wgpu::RequestAdapterOptions options{};
  options.compatibleSurface = m_Surface;

  const wgpu::Future adapter_future =
      m_Instance.RequestAdapter(&options, wgpu::CallbackMode::WaitAnyOnly, AdapterRequestCallback, &request_state);
  m_Instance.WaitAny(adapter_future, UINT64_MAX);

  if (request_state.Adapter == nullptr)
  {
    std::cerr << "Failed to request WebGPU adapter";
    if (!request_state.Message.empty())
    {
      std::cerr << ": " << request_state.Message;
    }
    std::cerr << '\n';
    return false;
  }

  m_Adapter = std::move(request_state.Adapter);
  return true;
}

bool WgpuGraphicsBackend::RequestDevice()
{
  DeviceRequestState request_state{};

  wgpu::DeviceDescriptor device_descriptor{};
  device_descriptor.SetDeviceLostCallback(wgpu::CallbackMode::AllowSpontaneous, DeviceLostCallback);
  device_descriptor.SetUncapturedErrorCallback(UncapturedErrorCallback);

  const wgpu::Future device_future = m_Adapter.RequestDevice(&device_descriptor, wgpu::CallbackMode::WaitAnyOnly,
                                                             DeviceRequestCallback, &request_state);
  m_Instance.WaitAny(device_future, UINT64_MAX);

  if (request_state.Device == nullptr)
  {
    std::cerr << "Failed to request WebGPU device";
    if (!request_state.Message.empty())
    {
      std::cerr << ": " << request_state.Message;
    }
    std::cerr << '\n';
    return false;
  }

  m_Device = std::move(request_state.Device);
  m_Queue = m_Device.GetQueue();
  return true;
}

bool WgpuGraphicsBackend::ConfigureSurface(std::uint32_t width, std::uint32_t height)
{
  wgpu::SurfaceCapabilities capabilities{};
  m_Surface.GetCapabilities(m_Adapter, &capabilities);

  if (capabilities.formatCount == 0 || capabilities.presentModeCount == 0 || capabilities.alphaModeCount == 0)
  {
    std::cerr << "Failed to configure WebGPU surface: missing surface capabilities\n";
    return false;
  }

  m_SurfaceConfiguration = {};
  m_SurfaceConfiguration.device = m_Device;
  m_SurfaceConfiguration.usage = wgpu::TextureUsage::RenderAttachment;
  m_SurfaceConfiguration.format = capabilities.formats[0];
  m_SurfaceConfiguration.alphaMode = capabilities.alphaModes[0];
  m_SurfaceConfiguration.presentMode = capabilities.presentModes[0];
  m_SurfaceConfiguration.width = width == 0 ? 1 : width;
  m_SurfaceConfiguration.height = height == 0 ? 1 : height;

  m_Surface.Configure(&m_SurfaceConfiguration);
  return true;
}

bool WgpuGraphicsBackend::AcquireSurfaceTexture()
{
  auto try_acquire = [this]()
  {
    m_SurfaceTexture = {};
    m_Surface.GetCurrentTexture(&m_SurfaceTexture);
    return m_SurfaceTexture.status;
  };

  wgpu::SurfaceGetCurrentTextureStatus status = try_acquire();
  if (status == wgpu::SurfaceGetCurrentTextureStatus::SuccessOptimal ||
      status == wgpu::SurfaceGetCurrentTextureStatus::SuccessSuboptimal)
  {
    if (m_SurfaceTexture.texture == nullptr)
    {
      std::cerr << "WebGPU surface acquisition succeeded without a texture\n";
      return false;
    }

    return true;
  }

  if (status == wgpu::SurfaceGetCurrentTextureStatus::Outdated || status == wgpu::SurfaceGetCurrentTextureStatus::Lost)
  {
    if (!ConfigureSurface(m_SurfaceConfiguration.width, m_SurfaceConfiguration.height))
    {
      return false;
    }

    status = try_acquire();
    if (status == wgpu::SurfaceGetCurrentTextureStatus::SuccessOptimal ||
        status == wgpu::SurfaceGetCurrentTextureStatus::SuccessSuboptimal)
    {
      if (m_SurfaceTexture.texture == nullptr)
      {
        std::cerr << "WebGPU surface reacquisition succeeded without a texture\n";
        return false;
      }

      return true;
    }
  }

  std::cerr << "Failed to acquire WebGPU surface texture: " << status << '\n';
  return false;
}

void WgpuGraphicsBackend::ResetTransientState()
{
  m_RenderPassEncoder = nullptr;
  m_CommandEncoder = nullptr;
  m_SurfaceTexture = {};
}
} // namespace Sidekick
