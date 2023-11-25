#include "kami/arguments.hpp"
#include "kami/linked_mesh.hpp"
#include "kami/linked_pool.hpp"
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

  // Load STL file
  microstl::MeshReaderHandler handler;
  std::cout << "Loading STL file" << std::endl;
  auto start_time = std::chrono::high_resolution_clock::now();
  if (microstl::Result result = loadSTL(handler, args.input);
      result != microstl::Result::Success) {
    std::cout << "Couldn't load file (" << args.input
              << "):" << microstl::getResultString(result) << std::endl;
    return -1;
  }
  auto end_time = std::chrono::high_resolution_clock::now();
  std::cout << "\tTook "
            << (end_time - start_time) / std::chrono::milliseconds(1)
            << " ms for " << handler.mesh.facets.size() << " faces."
            << std::endl;

  // Transform to the facet pool and make internal links
  std::cout << "Making Facet Pool" << std::endl;
  start_time = std::chrono::high_resolution_clock::now();
  kami::LinkedMeshPool pool(handler.mesh);
  pool.makeFacetPoolInternalLink();
  end_time = std::chrono::high_resolution_clock::now();
  std::cout << "\tTook "
            << (end_time - start_time) / std::chrono::milliseconds(1) << " ms"
            << std::endl;

  pool.printInformations();
  pool.unfold(args.max_depth);
  std::cout << pool << std::endl;
  auto bins = pool.slice();

  std::cout << "Exported " << bins.size() << " figure(s) as SVG" << std::endl;

  // Extract pattern
  std::cout << "Making Facet Pattern" << std::endl;
  start_time = std::chrono::high_resolution_clock::now();
  for (auto &bin : bins) {
    std::cout << "Export bin " << bin.id << " (" << bin.format.width << ", "
              << bin.format.height << ")" << std::endl;
    std::stringstream ss;
    ss << args.output << "_" << bin.id << ".svg";

    std::ofstream file(ss.str(), std::ios::out | std::ios::trunc);
    if (file.is_open()) {
      file << pool.getAsSVGString(bin, args.max_depth, args.resolution);
    }
    file.close();
  }
  end_time = std::chrono::high_resolution_clock::now();
  std::cout << "\tTook "
            << (end_time - start_time) / std::chrono::milliseconds(1) << " ms"
            << std::endl;

  return 0;
}