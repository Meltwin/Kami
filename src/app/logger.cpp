#include "kami/app/logger.hpp"

std::map<LogLevel, const char *> Logger::lvl_name =
    std::map<LogLevel, const char *>{{TRACE, "TRACE"}, {DEBUG, "DEBUG"},
                                     {INFO, "INFO"},   {WARN, "WARN"},
                                     {ERROR, "ERROR"}, {FATAL, "FATAL"}};

Logger::Logger(const char *log_name) : name(log_name) {}
