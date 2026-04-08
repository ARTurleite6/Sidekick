#include "Sidekick/Platform/OpenGLGraphicsContext.hpp"

#include "Sidekick/Core/GraphicsContext.hpp"
#include "Sidekick/Platform/OpenGLCommandList.hpp"
#include "Sidekick/Platform/OpenGLSwapchain.hpp"

#include <memory>
#include <utility>

namespace Sidekick
{
std::unique_ptr<Swapchain> OpenGLGraphicsContext::CreateSwapchain(SwapchainDescriptor&& desc)
{
  return std::make_unique<OpenGLSwapchain>(std::move(desc));
}

std::unique_ptr<CommandList> OpenGLGraphicsContext::CreateCommandList(CommandListDescriptor&& desc)
{
  return std::make_unique<OpenGLCommandList>(std::move(desc));
}

} // namespace Sidekick
