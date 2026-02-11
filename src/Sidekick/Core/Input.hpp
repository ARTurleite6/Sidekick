#pragma once

#include <unordered_set>
namespace Sidekick
{
class Input
{
public:
  static void Init();
  static void Update();

  // TODO: Create Key Codes
  static bool IsKeyDown(int key);
  static bool IsKeyPressed(int key);
  static void IsKeyReleased(int key);

private:
  static std::unordered_set<int> s_KeysDown;
  static std::unordered_set<int> s_KeysPressed;
  static std::unordered_set<int> s_KeysReleased;
};
} // namespace Sidekick
