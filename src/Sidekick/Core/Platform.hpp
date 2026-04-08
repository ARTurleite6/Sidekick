#pragma once

#include <memory>

namespace Sidekick
{
struct WindowDescriptor;
class Window;

class Platform
{
public:
  static std::unique_ptr<Platform> Create();

  virtual ~Platform() noexcept = default;

  virtual std::unique_ptr<Window> CreateWindow(WindowDescriptor&& desc) = 0;

  virtual void Update() = 0;
};
} // namespace Sidekick
