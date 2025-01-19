#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <cstdarg>

#include <cstdio>
#include <map>

enum LogLevel {
  TRACE = 0,
  DEBUG = 5,
  INFO = 10,
  WARN = 15,
  ERROR = 20,
  FATAL = 25
};

class Logger {
public:
  explicit Logger(const char *name);

#define LOG_FUNCTION(name, lvl)                                                \
  void name(const char *format, ...) const {                                   \
    va_list args;                                                              \
    va_start(args, format);                                                    \
    send_out(lvl, format, args);                                               \
    va_end(args);                                                              \
  }

  LOG_FUNCTION(trace, TRACE)
  LOG_FUNCTION(debug, DEBUG)
  LOG_FUNCTION(info, INFO)
  LOG_FUNCTION(warn, WARN)
  LOG_FUNCTION(error, ERROR)
  LOG_FUNCTION(fatal, FATAL)

  static void register_log_level(LogLevel lvl, const char *name) {
    lvl_name.emplace(std::pair(lvl, name));
  }

private:
  void send_out(const LogLevel lvl, const char *format, va_list list) const {
    fprintf(stdout, "[%6s][%-12s] ", get_log_level_name(lvl), name);
    vfprintf(stdout, format, list);
    fprintf(stdout, "\n");
  }

  static const char *get_log_level_name(const LogLevel lvl) {
    if (const auto it = lvl_name.find(lvl); it != lvl_name.end())
      return it->second;
    return "";
  }

private:
  static std::map<LogLevel, const char *> lvl_name;
  const char *name;
};

#endif // LOGGER_HPP
