#ifndef KAMI_LOGGING
#define KAMI_LOGGING

#include <chrono>

#define TIMED_UTILS                                                            \
  std::chrono::system_clock::time_point start;                                 \
  std::chrono::system_clock::time_point end;

#define TIMED_SECTION(title, exec)                                             \
  std::cout << title << std::endl;                                             \
  start = std::chrono::high_resolution_clock::now();                           \
  exec;                                                                        \
  end = std::chrono::high_resolution_clock::now();                             \
  std::cout << "\tTook " << (end - start) / std::chrono::milliseconds(1)       \
            << " ms" << std::endl;

#endif