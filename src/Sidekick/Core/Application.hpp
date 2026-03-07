#pragma once

#include <glm/ext/vector_float3.hpp>
#include <webgpu/webgpu.h>

#include "Sidekick/Core/Window.hpp"

#include <iostream>

namespace Sidekick
{
class Application
{
public:
  Application();

  void Run();

private:
  bool m_Running{true};
  Window m_Window;
};
} // namespace Sidekick
