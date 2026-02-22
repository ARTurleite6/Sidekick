#pragma once

#include <cstdint>

namespace Sidekick::Core {

enum class KeyCode : uint16_t {
  Unknown = 0,
  W,
  A,
  S,
  D,
  Q,
  E,
  Space,
  LeftShift,
  RightShift,
  Count,
};

}  // namespace Sidekick::Core
