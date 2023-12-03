#ifndef KAMI_ARGUMENTS
#define KAMI_ARGUMENTS

#include <cmath>
#include <cstring>
#include <iostream>
#include <ostream>
#include <sstream>
#include <string>

namespace kami::args {

inline void appHeader() {
  std::cout << " --- Kami, a paper pattern maker by Meltwin (2023) ---"
            << std::endl;
}

inline void printHelp() {
  std::cout << "Usage : kami -i <input file> -o <output file> [args]"
            << std::endl;
  std::cout << "Optional Arguments : " << std::endl;
  std::cout << "\t-s: rescale the figure in the world." << std::endl;
  std::cout
      << "\t-f: change the resolution of the output (to get thinner lines)"
      << std::endl;
  std::cout << "\t-d: maximum recursive depth (for debug purposes)"
            << std::endl;
  std::cout << "\t-h: show this help" << std::endl;
}

constexpr char ARG_INPUT[]{"-i"};
constexpr char ARG_OUTPUT[]{"-o"};
constexpr char ARG_WORLD_SCALING[]{"-s"};
constexpr char ARG_RESOLUTION[]{"-f"};
constexpr char ARG_MAX_DEPTH[]{"-d"};
constexpr char ARG_SVG_DEBUG[]{"-svgdbg"};
constexpr char ARG_HELP[]{"-h"};

enum class Arg { NONE, INPUT, OUTPUT, W_SCALING, RESOLUTION, MAX_DEPTH };

constexpr long NO_REC_LIMIT{-1};
struct Args {
  // IO
  std::string input = "";
  std::string output = "";

  // Scaling
  double world_scaling = 1.0;
  double resolution = 10.0;

  // Debug
  int max_depth = NO_REC_LIMIT;

  bool askHelp = false;
  bool svg_debug = false;

  friend std::ostream &operator<<(std::ostream &os, Args &args) {
    os << "Parameters : " << std::endl;
    os << "\tInput : " << args.input << std::endl;
    os << "\tOutput : " << args.output << "_X.svg" << std::endl;
    os << "\tScale : " << Args::printAsScale(args.world_scaling) << std::endl;
    os << "\tResolution : " << args.resolution << std::endl;
    os << "\tMax depth : " << args.max_depth << std::endl;
    return os;
  }

private:
  static std::string printAsScale(double scaling) {
    std::stringstream ss;
    if (scaling < 1) {
      int n = 0;
      do {
        scaling *= 10;
        n++;
      } while (scaling < 1);

      ss << scaling << ":" << std::pow(10, n);
    } else if (scaling > 1) {
      ss << scaling << ":" << 1;
    } else {
      ss << "1:1";
    }
    return ss.str();
  }
};

inline Args getArguments(int argc, char **argv) {
  Args args;
  Arg next = Arg::NONE;
  for (int i = 0; i < argc; i++) {
    char *arg = argv[i];

    // If taking next
    switch (next) {
    case Arg::INPUT:
      args.input = arg;
      break;
    case Arg::OUTPUT:
      args.output = arg;
      break;
    case Arg::W_SCALING:
      args.world_scaling = std::stod(arg);
      break;
    case Arg::RESOLUTION:
      args.resolution = std::stod(arg);
      break;
    case Arg::MAX_DEPTH:
      args.max_depth = std::stoi(arg);
      break;
    default:
      break;
    }
    next = Arg::NONE;

    // Get command
    if (strcmp(arg, ARG_INPUT) == 0)
      next = Arg::INPUT;
    else if (strcmp(arg, ARG_OUTPUT) == 0)
      next = Arg::OUTPUT;
    else if (strcmp(arg, ARG_WORLD_SCALING) == 0)
      next = Arg::W_SCALING;
    else if (strcmp(arg, ARG_RESOLUTION) == 0)
      next = Arg::RESOLUTION;
    else if (strcmp(arg, ARG_MAX_DEPTH) == 0)
      next = Arg::MAX_DEPTH;
    else if (strcmp(arg, ARG_HELP) == 0)
      args.askHelp = true;
    else if (strcmp(arg, ARG_SVG_DEBUG) == 0)
      args.svg_debug = true;
  }
  return args;
}

inline bool verifyArgs(const Args &args) {
  if (args.input.compare("") == 0) {
    printHelp();
    std::cout << "Error: No input file was given !" << std::endl;
    return true;
  } else if (args.output.compare("") == 0) {
    printHelp();
    std::cout << "Error: No output file was given !" << std::endl;
    return true;
  }
  return false;
}

} // namespace kami::args

#endif