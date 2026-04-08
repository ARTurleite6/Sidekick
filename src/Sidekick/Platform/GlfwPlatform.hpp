#pragma once

#include "Sidekick/Core/Platform.hpp"

#include <memory>

namespace Sidekick
{
class Window;
struct WindowDescriptor;

class GlfwPlatform final : public Platform
{
public:
  GlfwPlatform();
  ~GlfwPlatform() noexcept override;

  std::unique_ptr<Window> CreateWindow(WindowDescriptor&& desc) override;

  void Update() override;
};
} // namespace Sidekick
