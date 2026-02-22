#pragma once

#include <atomic>
#include <cstdint>
#include <format>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <utility>

namespace Sidekick::Core {

enum class LogLevel : std::uint8_t {
  Trace = 0,
  Info,
  Warn,
  Error,
  Off,
};

class Logger {
public:
  virtual ~Logger() = default;

  virtual void SetLevel(LogLevel level) = 0;
  virtual LogLevel GetLevel() const = 0;
  virtual void LogMessage(LogLevel level, std::string_view message) = 0;
};

class ConsoleLogger final : public Logger {
public:
  ConsoleLogger();

  void SetLevel(LogLevel level) override;
  LogLevel GetLevel() const override;
  void LogMessage(LogLevel level, std::string_view message) override;

private:
  std::atomic<LogLevel> m_level;
  std::mutex m_output_mutex;
};

class Log {
public:
  static void Initialize();
  static void Shutdown();

  static void SetLevel(LogLevel level);
  static LogLevel GetLevel();

  static void SetDefaultLogger(std::unique_ptr<Logger> logger);
  static Logger& GetDefaultLogger();

#ifndef NDEBUG
  template <typename... TArgs> static void Trace(std::format_string<TArgs...> format, TArgs&&... args) {
    LogLevelMessage(LogLevel::Trace, std::format(format, std::forward<TArgs>(args)...));
  }
#else
  template <typename... TArgs> static void Trace(std::format_string<TArgs...>, TArgs&&...) {}
#endif

  template <typename... TArgs> static void Info(std::format_string<TArgs...> format, TArgs&&... args) {
    LogLevelMessage(LogLevel::Info, std::format(format, std::forward<TArgs>(args)...));
  }

  template <typename... TArgs> static void Warn(std::format_string<TArgs...> format, TArgs&&... args) {
    LogLevelMessage(LogLevel::Warn, std::format(format, std::forward<TArgs>(args)...));
  }

  template <typename... TArgs> static void Error(std::format_string<TArgs...> format, TArgs&&... args) {
    LogLevelMessage(LogLevel::Error, std::format(format, std::forward<TArgs>(args)...));
  }

private:
  static void LogLevelMessage(LogLevel level, const std::string& message);
};

#ifndef NDEBUG
#define SK_TRACE(...) ::Sidekick::Core::Log::Trace(__VA_ARGS__)
#else
#define SK_TRACE(...) ((void)0)
#endif

#define SK_INFO(...) ::Sidekick::Core::Log::Info(__VA_ARGS__)
#define SK_WARN(...) ::Sidekick::Core::Log::Warn(__VA_ARGS__)
#define SK_ERROR(...) ::Sidekick::Core::Log::Error(__VA_ARGS__)

} // namespace Sidekick::Core
