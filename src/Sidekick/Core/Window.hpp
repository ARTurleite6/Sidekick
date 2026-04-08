#pragma once

#include <cstdint>
#include <string_view>

namespace Sidekick
{
struct WindowDescriptor
{
  std::string_view title{"Sidekick"};
  uint32_t width{1280};
  uint32_t height{720};
};

class Window
{
public:
  virtual ~Window() noexcept = default;
  virtual void* GetHandle() const = 0;

  virtual bool ShouldClose() const = 0;
};
} // namespace Sidekick
