#include "Sidekick/Core/Application.hpp"

#include <cstdlib>
#include <exception>
#include <iostream>

int main()
{
  try
  {
    Sidekick::Application app;
    app.Run();
    return EXIT_SUCCESS;
  }
  catch (const std::exception& exception)
  {
    std::cerr << exception.what() << '\n';
  }
  catch (...)
  {
    std::cerr << "Unhandled exception\n";
  }

  return EXIT_FAILURE;
}
