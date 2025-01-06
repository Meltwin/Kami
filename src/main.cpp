#include "kami/global/arguments.hpp"
#include "kami/global/logging.hpp"
#include "kami/mesh/linked_poly.hpp"
#include "kami/mesh/linked_pool.hpp"
#include "microstl/microstl.hpp"
#include <chrono>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

microstl::Result loadSTL(microstl::MeshReaderHandler &handler,
                         const std::string &file_path) {
  handler.forceNormals = true;
  handler.disableNormals = true;
  return microstl::Reader::readStlFile(file_path, handler);
}

int main(int argc, char **argv) {
  // Parse arguments + help
  kami::args::Args args = kami::args::getArguments(argc, argv);
  kami::args::appHeader();

  // If looking for help
  if (args.askHelp) {
    kami::args::printHelp();
    return 0;
  } else if (kami::args::verifyArgs(args)) {
    return -1;
  }

  std::cout << args;

  TIMED_UTILS;

  // Load STL file
  microstl::MeshReaderHandler handler;
  TIMED_SECTION("Loading STL file", {
    if (microstl::Result result = loadSTL(handler, args.input);
        result != microstl::Result::Success) {
      std::cout << "Couldn't load file (" << args.input
                << "):" << microstl::getResultString(result) << std::endl;
      return -1;
    }
    std::cout << "\tLoaded " << handler.mesh.facets.size() << " facets"
              << std::endl;
  });

  // Make the linking pool
  auto pool = kami::LinkedMeshPool(handler.mesh);

  // Informations for debug
  printSectionHeader("Raw Mesh Properties (Link + Merge)");
  pool.printInformations();
  std::cout << pool << std::endl;

  // Unfold the linked mesh
  pool.unfold(args.max_depth);
  printSectionHeader("Unfold Mesh Properties");
  std::cout << pool << std::endl;

  // Change the figure scale in the world
  pool.scaleFigure(args.world_scaling);

  // Slice the linked mesh in multiple parts
  kami::MeshBinVector bins = pool.slice();

  auto make_file_name = [&args](const std::string &suffix) {
    std::stringstream ss;
    ss << args.output << "_" << suffix << ".svg";
    return ss.str();
  };

// Projections
#define projectionStep(side, function)                                         \
  {                                                                            \
    std::stringstream ss;                                                      \
    ss << "Export " << side;                                                   \
    printStepHeader(ss.str());                                                 \
    file =                                                                     \
        std::ofstream(make_file_name(side), std::ios::out | std::ios::trunc);  \
    if (file.is_open()) {                                                      \
      file << pool.function(args);                                             \
    }                                                                          \
    file.close();                                                              \
  }

  printSectionHeader("Projections to SVG");
  std::ofstream file;
  projectionStep("top", projectOnTop);
  projectionStep("bottom", projectOnBottom);
  projectionStep("front", projectOnFront);
  projectionStep("back", projectOnBack);
  projectionStep("right", projectOnRight);
  projectionStep("left", projectOnLeft);

  // Extract pattern
  printSectionHeader("Exporting to SVG");
  for (auto &bin : bins) {
    printStepHeader("Export bin");
    std::cout << bin << std::endl;
    std::stringstream ss;
    ss << args.output << "_" << bin.id + 1 << ".svg";

    std::ofstream file(ss.str(), std::ios::out | std::ios::trunc);
    if (file.is_open()) {
      file << pool.getAsSVGString(bin, args);
    }
    file.close();
  }

  std::cout << std::endl << std::endl;

  return 0;
}