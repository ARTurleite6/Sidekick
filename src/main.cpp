#include "Sidekick/Core/Application.h"

int main() {
  Sidekick::Core::Application app;
  if (!app.Initialize()) {
    return 1;
  }
  app.Run();
  app.Shutdown();
  return 0;
}
