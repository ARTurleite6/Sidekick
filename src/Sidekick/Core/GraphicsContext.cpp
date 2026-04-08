#include "Sidekick/Core/GraphicsContext.hpp"

#include "Sidekick/Platform/OpenGLGraphicsContext.hpp"

#include <memory>

namespace Sidekick
{
std::unique_ptr<GraphicsContext> GraphicsContext::Create()
{
  return std::make_unique<OpenGLGraphicsContext>();
}
} // namespace Sidekick
