#pragma once

#include <cstdint>
#include <string>
#include <string_view>

namespace Sidekick
{
enum class EventType : std::uint16_t
{
  None = 0,
  WindowClose,
  KeyPressed,
  KeyReleased,
};

class Event
{
public:
  virtual ~Event() = default;
  virtual EventType GetType() const noexcept = 0;
  virtual std::string_view GetName() const = 0;
  virtual std::string ToString() const { return std::string{GetName()}; }
};
} // namespace Sidekick
