#include "Sidekick/Core/Application.h"
#include "Sidekick/Core/Log.h"

int main() {
  Sidekick::Core::Log::Initialize();

  Sidekick::Core::Application app;
  if (!app.Initialize()) {
    Sidekick::Core::Log::Shutdown();
    return 1;
  }
  app.Run();
  app.Shutdown();

  Sidekick::Core::Log::Shutdown();
  return 0;
}
