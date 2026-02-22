#pragma once

#include "Sidekick/Core/Log.h"

#include <cstdlib>
#include <format>
#include <string_view>
#include <utility>

namespace Sidekick::Core::Assert {

[[noreturn]] inline void HandleFailure(std::string_view check, const char* file, int line) {
  SK_ERROR("Assertion '{}' failed at {}:{}", check, file, line);
  std::abort();
}

template <typename... TArgs>
[[noreturn]] inline void HandleFailure(std::string_view check, const char* file, int line,
                                       std::format_string<TArgs...> format, TArgs&&... args) {
  (void)check;
  (void)file;
  (void)line;

  SK_ERROR("{}", std::format(format, std::forward<TArgs>(args)...));
  std::abort();
}

} // namespace Sidekick::Core::Assert

// Requested behavior: assertions are disabled on debug builds and enabled when NDEBUG is set.
#ifdef NDEBUG
#define SK_ASSERT(check, ...)                                                                                          \
  do {                                                                                                                 \
    if (!(check)) {                                                                                                    \
      ::Sidekick::Core::Assert::HandleFailure(#check, __FILE__, __LINE__ __VA_OPT__(, __VA_ARGS__));                   \
    }                                                                                                                  \
  } while (false)
#else
#define SK_ASSERT(check, ...) ((void)0)
#endif
