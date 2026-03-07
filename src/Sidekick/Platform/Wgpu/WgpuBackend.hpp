#pragma once

#include "Sidekick/Renderer/GraphicsBackend.hpp"

#include "webgpu/webgpu_cpp.h"

#include <cstdint>

namespace Sidekick
{
class WgpuGraphicsBackend final : public GraphicsBackend
{
public:
  WgpuGraphicsBackend() = default;

private:
  bool OnInit(const GraphicsContextDescriptor& descriptor) override;

  void OnBeginFrame() override;
  void OnBeginRenderPass(const RenderPassDescriptor& descriptor) override;
  void OnEndRenderPass() override;
  void OnEndFrame() override;
  void OnResize(std::uint32_t width, std::uint32_t height) override;

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

} // namespace Sidekick
