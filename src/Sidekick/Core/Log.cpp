#include "Sidekick/Core/Log.h"

#include <chrono>
#include <cstdio>
#include <ctime>
#include <exception>
#include <mutex>
#include <utility>

#if defined(_WIN32)
#include <io.h>
#else
#include <unistd.h>
#endif

#if __has_include(<print>)
#include <print>
#endif

namespace Sidekick::Core {

namespace {

std::unique_ptr<Logger> s_default_logger;
std::mutex s_logger_mutex;

const char* ToString(LogLevel level) {
  switch (level) {
  case LogLevel::Trace:
    return "TRACE";
  case LogLevel::Info:
    return "INFO";
  case LogLevel::Warn:
    return "WARN";
  case LogLevel::Error:
    return "ERROR";
  case LogLevel::Off:
    return "OFF";
  default:
    return "UNKNOWN";
  }
}

bool IsEnabled(LogLevel message_level, LogLevel configured_level) {
  if (configured_level == LogLevel::Off) {
    return false;
  }
  return static_cast<std::uint8_t>(message_level) >= static_cast<std::uint8_t>(configured_level);
}

std::tm GetLocalTime(std::time_t timestamp) {
  std::tm local_time = {};
#if defined(_WIN32)
  localtime_s(&local_time, &timestamp);
#else
  localtime_r(&timestamp, &local_time);
#endif
  return local_time;
}

bool SupportsColor(std::FILE* stream) {
#if defined(_WIN32)
  return _isatty(_fileno(stream)) != 0;
#else
  return isatty(fileno(stream)) != 0;
#endif
}

const char* LevelColor(LogLevel level) {
  switch (level) {
  case LogLevel::Trace:
    return "\033[90m";
  case LogLevel::Info:
    return "\033[36m";
  case LogLevel::Warn:
    return "\033[33m";
  case LogLevel::Error:
    return "\033[31m";
  case LogLevel::Off:
    return "\033[0m";
  default:
    return "\033[0m";
  }
}

std::string BuildPrefix(LogLevel level, bool colorize) {
  const auto now = std::chrono::system_clock::now();
  const std::time_t now_time = std::chrono::system_clock::to_time_t(now);
  const std::tm local_time = GetLocalTime(now_time);

  if (colorize) {
    return std::format("[{0:02d}:{1:02d}:{2:02d}] [{3}{4}\033[0m] ", local_time.tm_hour, local_time.tm_min,
                       local_time.tm_sec, LevelColor(level), ToString(level));
  }

  return std::format("[{0:02d}:{1:02d}:{2:02d}] [{3}] ", local_time.tm_hour, local_time.tm_min, local_time.tm_sec,
                     ToString(level));
}

std::FILE* StreamForLevel(LogLevel level) {
  if (level == LogLevel::Warn || level == LogLevel::Error) {
    return stderr;
  }
  return stdout;
}

} // namespace

ConsoleLogger::ConsoleLogger()
#ifdef NDEBUG
    : m_level(LogLevel::Info)
#else
    : m_level(LogLevel::Trace)
#endif
{
}

void ConsoleLogger::SetLevel(LogLevel level) { m_level.store(level, std::memory_order_relaxed); }

LogLevel ConsoleLogger::GetLevel() const { return m_level.load(std::memory_order_relaxed); }

void ConsoleLogger::LogMessage(LogLevel level, std::string_view message) {
  if (!IsEnabled(level, GetLevel())) {
    return;
  }

  std::FILE* stream = StreamForLevel(level);
  const bool colorize = SupportsColor(stream);
  const std::string line = std::format("{}{}", BuildPrefix(level, colorize), message);
  std::lock_guard<std::mutex> lock(m_output_mutex);

#if defined(__cpp_lib_print) && __cpp_lib_print >= 202207L
  if (stream == stderr) {
    std::println(stderr, "{}", line);
  } else {
    std::println(stdout, "{}", line);
  }
#else
  std::fwrite(line.data(), 1, line.size(), stream);
  std::fputc('\n', stream);
  std::fflush(stream);
#endif
}

void Log::Initialize() {
  std::lock_guard<std::mutex> lock(s_logger_mutex);
  if (!s_default_logger) {
    s_default_logger = std::make_unique<ConsoleLogger>();
  }
}

void Log::Shutdown() {
  std::lock_guard<std::mutex> lock(s_logger_mutex);
  s_default_logger.reset();
}

void Log::SetLevel(LogLevel level) {
  std::lock_guard<std::mutex> lock(s_logger_mutex);
  if (!s_default_logger) {
    s_default_logger = std::make_unique<ConsoleLogger>();
  }
  s_default_logger->SetLevel(level);
}

LogLevel Log::GetLevel() {
  std::lock_guard<std::mutex> lock(s_logger_mutex);
  if (!s_default_logger) {
    s_default_logger = std::make_unique<ConsoleLogger>();
  }
  return s_default_logger->GetLevel();
}

void Log::SetDefaultLogger(std::unique_ptr<Logger> logger) {
  std::lock_guard<std::mutex> lock(s_logger_mutex);
  if (!logger) {
    s_default_logger = std::make_unique<ConsoleLogger>();
    return;
  }
  s_default_logger = std::move(logger);
}

Logger& Log::GetDefaultLogger() {
  std::lock_guard<std::mutex> lock(s_logger_mutex);
  if (!s_default_logger) {
    s_default_logger = std::make_unique<ConsoleLogger>();
  }
  return *s_default_logger;
}

void Log::LogLevelMessage(LogLevel level, const std::string& message) {
  std::lock_guard<std::mutex> lock(s_logger_mutex);
  if (!s_default_logger) {
    s_default_logger = std::make_unique<ConsoleLogger>();
  }

  try {
    s_default_logger->LogMessage(level, message);
  } catch (const std::exception& exception) {
    const std::string fallback = std::format("[LOGGING ERROR] {}", exception.what());
    std::fwrite(fallback.data(), 1, fallback.size(), stderr);
    std::fputc('\n', stderr);
  } catch (...) {
    static constexpr std::string_view fallback = "[LOGGING ERROR] Unknown logging failure";
    std::fwrite(fallback.data(), 1, fallback.size(), stderr);
    std::fputc('\n', stderr);
  }
}

} // namespace Sidekick::Core
