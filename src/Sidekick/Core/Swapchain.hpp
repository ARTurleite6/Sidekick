#pragma once

#include <cstdint>

namespace Sidekick
{
struct SwapchainDescriptor
{
  void* window_handle;
  uint32_t size_x, size_y;
};

class Swapchain
{
public:
  virtual ~Swapchain() noexcept = default;

  virtual void Present() = 0;
};

}
