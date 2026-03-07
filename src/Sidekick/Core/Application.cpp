#include "Sidekick/Core/Application.hpp"

namespace Sidekick
{
namespace
{
WindowDescriptor CreateDefaultWindowDescriptor()
{
  return {
      .width = 1280,
      .height = 720,
      .title = "sidekick",
  };
}
} // namespace

Application::Application() : m_Window(CreateDefaultWindowDescriptor())
{
}

void Application::Run()
{
  if (!m_Window.IsValid())
  {
    return;
  }

  const glm::vec3 clear_color = {0.04F, 0.05F, 0.08F};
  std::cout << "Engine bootstrap color: " << clear_color.x << ", " << clear_color.y << ", " << clear_color.z << '\n';

  WGPUInstanceDescriptor instance_descriptor{};
  WGPUInstance dawn_instance = wgpuCreateInstance(&instance_descriptor);
  if (dawn_instance == nullptr)
  {
    std::cerr << "Failed to create Dawn instance\n";
    return;
  }

  while (m_Running)
  {
    if (m_Window.ShouldClose())
    {
      m_Running = false;
      break;
    }

    m_Window.PollEvents();
  }

  wgpuInstanceRelease(dawn_instance);
}
} // namespace Sidekick
