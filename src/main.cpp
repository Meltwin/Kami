#include "kami/arguments.hpp"
#include "kami/bin_packing.hpp"
#include "kami/linked_mesh.hpp"
#include "kami/linked_pool.hpp"
#include "kami/logging.hpp"
#include "microstl/microstl.hpp"
#include <chrono>
#include <fstream>
#include <iostream>
#include <sstream>

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
  kami::LinkedMeshPool pool(0);
  TIMED_SECTION("Making & Linking pool", {
    pool = kami::LinkedMeshPool(handler.mesh);
    pool.makeFacetPoolInternalLink();
  });
  pool.printInformations();

  // Unfold the linked mesh
  TIMED_SECTION("Unfolding the linked mesh", pool.unfold(args.max_depth));
  std::cout << pool << std::endl;

  // Change the figure scale in the world
  if (args.world_scaling != 1.f) {
    TIMED_SECTION("Rescaling the mesh", pool.scaleFigure(args.world_scaling));
  }

  // Slice the linked mesh in multiple parts
  kami::MeshBinVector bins(0, kami::MeshBin(kami::bin::PaperA<4>()));
  TIMED_SECTION("Slicing the linked mesh", bins = pool.slice());

  // Extract pattern
  std::cout << "Exporting " << bins.size() << " figure(s) as SVG" << std::endl;
  for (auto &bin : bins) {
    std::cout << "\tExport bin " << bin << std::endl;
    std::stringstream ss;
    ss << args.output << "_" << bin.id << ".svg";

    std::ofstream file(ss.str(), std::ios::out | std::ios::trunc);
    if (file.is_open()) {
      file << pool.getAsSVGString(bin, args.max_depth, args.resolution);
    }
    file.close();
  }

  return 0;
}