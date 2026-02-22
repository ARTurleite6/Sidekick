#include "Sidekick/Renderer/Backend/GraphicsBackend.h"

#include "Sidekick/Renderer/Backend/WgpuBackend.h"

namespace Sidekick::Renderer::Backend {

std::unique_ptr<GraphicsBackend> CreateGraphicsBackend(GraphicsBackendType type) {
  switch (type) {
  case GraphicsBackendType::Wgpu:
    return std::make_unique<WgpuBackend>();
  default:
    return nullptr;
  }
}

} // namespace Sidekick::Renderer::Backend
