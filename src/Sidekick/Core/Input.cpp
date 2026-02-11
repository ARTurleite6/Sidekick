#include "Sidekick/Core/Input.hpp"

namespace Sidekick
{
std::unordered_set<int> Input::s_KeysDown{};
std::unordered_set<int> Input::s_KeysPressed{};
std::unordered_set<int> Input::s_KeysReleased{};
} // namespace Sidekick
