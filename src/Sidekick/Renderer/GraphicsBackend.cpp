#include "Sidekick/Renderer/GraphicsBackend.hpp"

#include "Sidekick/Platform/Wgpu/WgpuBackend.hpp"

#include <cassert>
#include <cstdint>
#include <memory>
#include <utility>

namespace Sidekick
{
std::unique_ptr<GraphicsBackend> GraphicsBackend::Create()
{
  return std::make_unique<WgpuGraphicsBackend>();
}

bool GraphicsBackend::Init(const GraphicsContextDescriptor& descriptor)
{
  assert(!m_Initialized);
  if (m_Initialized)
  {
    return false;
  }

  m_Initialized = OnInit(descriptor);
  if (!m_Initialized)
  {
    return false;
  }

  return true;
}

void GraphicsBackend::BeginFrame()
{
  assert(m_Initialized);
  assert(!m_FrameActive);
  assert(!m_RenderPassActive);

  OnBeginFrame();
  m_FrameActive = true;
}

void GraphicsBackend::BeginRenderPass(const RenderPassDescriptor& descriptor)
{
  assert(m_Initialized);
  assert(m_FrameActive);
  assert(!m_RenderPassActive);

  OnBeginRenderPass(descriptor);
  m_RenderPassActive = true;
}

void GraphicsBackend::EndRenderPass()
{
  assert(m_Initialized);
  assert(m_FrameActive);
  assert(m_RenderPassActive);

  OnEndRenderPass();
  m_RenderPassActive = false;
}

void GraphicsBackend::EndFrame()
{
  assert(m_Initialized);
  assert(m_FrameActive);
  assert(!m_RenderPassActive);

  OnEndFrame();
  m_FrameActive = false;
}

void GraphicsBackend::Resize(uint32_t width, uint32_t height)
{
  assert(m_Initialized);
  assert(!m_FrameActive);
  assert(!m_RenderPassActive);

  OnResize(width, height);
}
} // namespace Sidekick
