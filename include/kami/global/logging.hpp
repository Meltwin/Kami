#ifndef KAMI_LOGGING
#define KAMI_LOGGING

#include <chrono>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <string>
#include <utility>

// ----------------------------------------------------------------------------
// Headers function
// ----------------------------------------------------------------------------

#define TITLE_WIDTH 80
#define TITLE_PAD_CHAR '-'
#define SECTION_WIDTH 60
#define SECTION_PAD_CHAR '='

inline std::pair<int, int> computePadding(int input_width, int output_width) {
  return std::pair<int, int>{std::ceil((output_width - input_width) / 2),
                             std::floor((output_width - input_width) / 2)};
}

inline void printTitle(std::string title) {
  auto pair = computePadding(title.length() + 2, TITLE_WIDTH);
  std::cout << std::string(pair.first, TITLE_PAD_CHAR) << " " << title << " "
            << std::string(pair.second, TITLE_PAD_CHAR) << std::endl;
}

inline void printSectionHeader(std::string title) {
  auto pairI = computePadding(title.length() + 2, SECTION_WIDTH);
  auto pairO = computePadding(SECTION_WIDTH, TITLE_WIDTH);
  std::cout << std::string(pairO.first, TITLE_PAD_CHAR);
  std::cout << std::string(pairI.first, SECTION_PAD_CHAR);
  std::cout << " " << title << " ";
  std::cout << std::string(pairI.second, SECTION_PAD_CHAR);
  std::cout << std::string(pairO.second, TITLE_PAD_CHAR);
  std::cout << std::endl;
}

inline void printStepHeader(std::string title) {
  std::cout << "====> " << title << std::endl;
}

// ----------------------------------------------------------------------------
// Timing utils
// ----------------------------------------------------------------------------

#define TIMED_UTILS                                                            \
  std::chrono::system_clock::time_point start;                                 \
  std::chrono::system_clock::time_point end;

#define TIMED_SECTION(title, exec)                                             \
  printSectionHeader(title);                                                   \
  start = std::chrono::high_resolution_clock::now();                           \
  exec;                                                                        \
  end = std::chrono::high_resolution_clock::now();                             \
  std::cout << std::endl                                                       \
            << "\t... Took " << (end - start) / std::chrono::milliseconds(1)   \
            << " ms" << std::endl;

#endif