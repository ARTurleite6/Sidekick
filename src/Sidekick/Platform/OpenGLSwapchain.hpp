#pragma once

#include "Sidekick/Core/Swapchain.hpp"

struct GLFWwindow;

namespace Sidekick
{
class OpenGLSwapchain : public Swapchain
{
public:
  OpenGLSwapchain(SwapchainDescriptor&& desc);

  void Present() override;

private:
  // TODO: check how to solve this in the future
  GLFWwindow* m_Window;
};

} // namespace Sidekick
