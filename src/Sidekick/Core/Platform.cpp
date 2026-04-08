#include "Sidekick/Platform/GlfwPlatform.hpp"

#include "Sidekick/Core/Platform.hpp"

#include <memory>

namespace Sidekick
{
std::unique_ptr<Platform> Platform::Create()
{
  return std::make_unique<GlfwPlatform>();
}
} // namespace Sidekick
