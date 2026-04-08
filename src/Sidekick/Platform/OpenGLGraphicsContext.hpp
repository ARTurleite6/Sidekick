#pragma once

#include "Sidekick/Core/GraphicsContext.hpp"

#include <memory>

struct GLFWwindow;

namespace Sidekick
{

class OpenGLGraphicsContext : public GraphicsContext
{
public:
  std::unique_ptr<Swapchain> CreateSwapchain(SwapchainDescriptor&& desc) override;

  std::unique_ptr<CommandList> CreateCommandList(CommandListDescriptor&& desc) override;
};
} // namespace Sidekick
