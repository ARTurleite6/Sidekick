#pragma once

#include <memory>

namespace Sidekick
{
class Swapchain;
struct SwapchainDescriptor;

class CommandList;
struct CommandListDescriptor;

class GraphicsContext
{
public:
  static std::unique_ptr<GraphicsContext> Create();

  virtual ~GraphicsContext() noexcept = default;

  virtual std::unique_ptr<Swapchain> CreateSwapchain(SwapchainDescriptor&& desc) = 0;
  virtual std::unique_ptr<CommandList> CreateCommandList(CommandListDescriptor&& desc) = 0;
};
} // namespace Sidekick
