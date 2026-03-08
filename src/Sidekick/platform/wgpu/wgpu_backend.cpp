#include "sidekick/platform/wgpu/wgpu_backend.hpp"

#include "sidekick/renderer/graphics_backend.hpp"

#include <webgpu/webgpu_cpp.h>
#include <webgpu/webgpu_cpp_print.h>
#include <webgpu/webgpu_glfw.h>

#include <cassert>
#include <cstdint>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>

namespace sidekick
{
namespace
{
constexpr wgpu::TextureFormat PREFERRED_SURFACE_FORMATS[]{
    wgpu::TextureFormat::BGRA8Unorm,
    wgpu::TextureFormat::BGRA8UnormSrgb,
    wgpu::TextureFormat::RGBA8Unorm,
    wgpu::TextureFormat::RGBA8UnormSrgb,
};

constexpr wgpu::LoadOp to_wgpu(load_op load_operation)
{
  switch (load_operation)
  {
  case load_op::load:
    return wgpu::LoadOp::Load;
  case load_op::clear:
    return wgpu::LoadOp::Clear;
  }

  return wgpu::LoadOp::Undefined;
}

constexpr wgpu::StoreOp to_wgpu(store_op store_operation)
{
  switch (store_operation)
  {
  case store_op::discard:
    return wgpu::StoreOp::Discard;
  case store_op::store:
    return wgpu::StoreOp::Store;
  }

  return wgpu::StoreOp::Undefined;
}

constexpr wgpu::Color to_wgpu(const clear_color& clear_value)
{
  return {.r = clear_value.r, .g = clear_value.g, .b = clear_value.b, .a = clear_value.a};
}

struct adapter_request_state
{
  wgpu::Adapter adapter;
  std::string message;
};

struct device_request_state
{
  wgpu::Device device;
  std::string message;
};

void adapter_request_callback(wgpu::RequestAdapterStatus status, wgpu::Adapter adapter, wgpu::StringView message,
                              adapter_request_state* state)
{
  if (status != wgpu::RequestAdapterStatus::Success)
  {
    std::ostringstream stream;
    stream << "status=" << status;
    if (message.data != nullptr)
    {
      stream << " message=" << message;
    }
    state->message = stream.str();
    return;
  }

  state->adapter = std::move(adapter);
}

void device_request_callback(wgpu::RequestDeviceStatus status, wgpu::Device device, wgpu::StringView message,
                             device_request_state* state)
{
  if (status != wgpu::RequestDeviceStatus::Success)
  {
    std::ostringstream stream;
    stream << "status=" << status;
    if (message.data != nullptr)
    {
      stream << " message=" << message;
    }
    state->message = stream.str();
    return;
  }

  state->device = std::move(device);
}

void uncaptured_error_callback(const wgpu::Device& device [[maybe_unused]], wgpu::ErrorType error_type,
                               wgpu::StringView message)
{
  std::cerr << "Uncaptured WebGPU error (" << error_type << "): " << message << '\n';
}

void device_lost_callback(const wgpu::Device& device [[maybe_unused]], wgpu::DeviceLostReason reason,
                          wgpu::StringView message)
{
  std::cerr << "WebGPU device lost (" << reason << "): " << message << '\n';
}

template <typename Value> bool contains(const Value* values, size_t count, Value value)
{
  for (size_t index = 0; index < count; ++index)
  {
    if (values[index] == value)
    {
      return true;
    }
  }

  return false;
}

wgpu::TextureFormat choose_surface_format(const wgpu::SurfaceCapabilities& capabilities)
{
  for (wgpu::TextureFormat format : PREFERRED_SURFACE_FORMATS)
  {
    if (contains(capabilities.formats, capabilities.formatCount, format))
    {
      return format;
    }
  }

  return capabilities.formats[0];
}

wgpu::PresentMode choose_present_mode(const wgpu::SurfaceCapabilities& capabilities)
{
  if (contains(capabilities.presentModes, capabilities.presentModeCount, wgpu::PresentMode::Fifo))
  {
    return wgpu::PresentMode::Fifo;
  }

  return capabilities.presentModes[0];
}

wgpu::CompositeAlphaMode choose_alpha_mode(const wgpu::SurfaceCapabilities& capabilities)
{
  if (contains(capabilities.alphaModes, capabilities.alphaModeCount, wgpu::CompositeAlphaMode::Auto))
  {
    return wgpu::CompositeAlphaMode::Auto;
  }

  return capabilities.alphaModes[0];
}
} // namespace

bool wgpu_graphics_backend::on_init(const graphics_context_descriptor& descriptor)
{
  assert(descriptor.native_window);

  static constexpr auto TIMED_WAIT_ANY = wgpu::InstanceFeatureName::TimedWaitAny;
  wgpu::InstanceDescriptor instance_descriptor{
      .requiredFeatureCount = 1,
      .requiredFeatures = &TIMED_WAIT_ANY,
  };

  m_instance = wgpu::CreateInstance(&instance_descriptor);
  assert(m_instance && "Failed to create WebGPU instance\n");

  m_surface = wgpu::glfw::CreateSurfaceForWindow(m_instance, descriptor.native_window);
  assert(m_surface && "Failed to create WebGPU surface for GLFW window\n");

  assert(request_adapter());
  assert(request_device());

  return configure_surface(descriptor.framebuffer_extent.width, descriptor.framebuffer_extent.height);
}

bool wgpu_graphics_backend::on_begin_frame()
{
  assert(m_frame_context.command_encoder == nullptr);
  assert(m_frame_context.render_pass_encoder == nullptr);

  if (!acquire_surface_texture())
  {
    return false;
  }

  m_frame_context.command_encoder = m_device.CreateCommandEncoder();
  return m_frame_context.command_encoder != nullptr;
}

void wgpu_graphics_backend::on_begin_render_pass(const render_pass_descriptor& descriptor)
{
  assert(m_frame_context.command_encoder != nullptr);
  assert(m_frame_context.render_pass_encoder == nullptr);
  assert(m_frame_context.surface_texture.texture != nullptr);

  const wgpu::TextureView texture_view = m_frame_context.surface_texture.texture.CreateView();

  wgpu::RenderPassColorAttachment color_attachment{
      .view = texture_view,
      .loadOp = to_wgpu(descriptor.color_attachment.load_operation),
      .storeOp = to_wgpu(descriptor.color_attachment.store_operation),
      .clearValue = to_wgpu(descriptor.color_attachment.clear_value),
  };

  wgpu::RenderPassDescriptor render_pass_descriptor{
      .colorAttachmentCount = 1,
      .colorAttachments = &color_attachment,
  };

  m_frame_context.render_pass_encoder = m_frame_context.command_encoder.BeginRenderPass(&render_pass_descriptor);
}

void wgpu_graphics_backend::on_end_render_pass()
{
  assert(m_frame_context.render_pass_encoder != nullptr);

  m_frame_context.render_pass_encoder.End();
  m_frame_context.render_pass_encoder = nullptr;
}

void wgpu_graphics_backend::on_end_frame()
{
  assert(m_frame_context.command_encoder != nullptr);
  assert(m_frame_context.render_pass_encoder == nullptr);
  assert(m_frame_context.surface_texture.texture != nullptr);

  const wgpu::CommandBuffer command_buffer = m_frame_context.command_encoder.Finish();

  m_frame_context.command_encoder = nullptr;
  m_queue.Submit(1, &command_buffer);
  m_surface.Present();

  m_frame_context.reset();
}

bool wgpu_graphics_backend::on_resize(std::uint32_t width, std::uint32_t height)
{
  assert(m_frame_context.command_encoder == nullptr);
  assert(m_frame_context.render_pass_encoder == nullptr);

  return configure_surface(width, height);
}

bool wgpu_graphics_backend::request_adapter()
{
  adapter_request_state request_state{};

  wgpu::RequestAdapterOptions options{
      .compatibleSurface = m_surface,
  };

  const wgpu::Future adapter_future =
      m_instance.RequestAdapter(&options, wgpu::CallbackMode::WaitAnyOnly, adapter_request_callback, &request_state);
  m_instance.WaitAny(adapter_future, UINT64_MAX);

  if (request_state.adapter == nullptr)
  {
    std::cerr << "Failed to request WebGPU adapter";
    if (!request_state.message.empty())
    {
      std::cerr << ": " << request_state.message;
    }
    std::cerr << '\n';
    return false;
  }

  m_adapter = request_state.adapter;
  return true;
}

bool wgpu_graphics_backend::request_device()
{
  device_request_state request_state{};

  wgpu::DeviceDescriptor device_descriptor{};
  device_descriptor.SetDeviceLostCallback(wgpu::CallbackMode::AllowSpontaneous, device_lost_callback);
  device_descriptor.SetUncapturedErrorCallback(uncaptured_error_callback);

  const wgpu::Future device_future = m_adapter.RequestDevice(&device_descriptor, wgpu::CallbackMode::WaitAnyOnly,
                                                             device_request_callback, &request_state);
  m_instance.WaitAny(device_future, UINT64_MAX);

  if (request_state.device == nullptr)
  {
    std::cerr << "Failed to request WebGPU device";
    if (!request_state.message.empty())
    {
      std::cerr << ": " << request_state.message;
    }
    std::cerr << '\n';
    return false;
  }

  m_device = request_state.device;
  m_queue = m_device.GetQueue();
  return true;
}

bool wgpu_graphics_backend::configure_surface(std::uint32_t width, std::uint32_t height)
{
  wgpu::SurfaceCapabilities capabilities{};
  m_surface.GetCapabilities(m_adapter, &capabilities);

  if (capabilities.formatCount == 0 || capabilities.presentModeCount == 0 || capabilities.alphaModeCount == 0)
  {
    std::cerr << "WebGPU surface capabilities are incomplete\n";
    return false;
  }

  m_surface_configuration = {
      .device = m_device,
      .format = choose_surface_format(capabilities),
      .usage = wgpu::TextureUsage::RenderAttachment,
      .width = width == 0 ? 1 : width,
      .height = height == 0 ? 1 : height,
      .alphaMode = choose_alpha_mode(capabilities),
      .presentMode = choose_present_mode(capabilities),
  };

  m_surface.Configure(&m_surface_configuration);
  return true;
}

bool wgpu_graphics_backend::acquire_surface_texture()
{
  auto try_acquire = [this]()
  {
    m_frame_context.surface_texture = {};
    m_surface.GetCurrentTexture(&m_frame_context.surface_texture);
    return m_frame_context.surface_texture.status;
  };

  wgpu::SurfaceGetCurrentTextureStatus status = try_acquire();
  if (status == wgpu::SurfaceGetCurrentTextureStatus::SuccessOptimal ||
      status == wgpu::SurfaceGetCurrentTextureStatus::SuccessSuboptimal)
  {
    return true;
  }

  if (status == wgpu::SurfaceGetCurrentTextureStatus::Outdated || status == wgpu::SurfaceGetCurrentTextureStatus::Lost)
  {
    if (!configure_surface(m_surface_configuration.width, m_surface_configuration.height))
    {
      return false;
    }

    status = try_acquire();
    if (status == wgpu::SurfaceGetCurrentTextureStatus::SuccessOptimal ||
        status == wgpu::SurfaceGetCurrentTextureStatus::SuccessSuboptimal)
    {
      return true;
    }
  }

  if (status == wgpu::SurfaceGetCurrentTextureStatus::Timeout)
  {
    std::cerr << "Timed out acquiring WebGPU surface texture; skipping frame\n";
    return false;
  }

  std::cerr << "Failed to acquire WebGPU surface texture: " << status << '\n';
  return false;
}
} // namespace sidekick
