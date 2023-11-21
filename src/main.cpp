#include "kami/linked_mesh.hpp"
#include "kami/linked_pool.hpp"
#include "microstl/microstl.hpp"
#include <chrono>
#include <fstream>
#include <iostream>

microstl::Result loadSTL(microstl::MeshReaderHandler &handler,
                         const std::string &file_path) {
  return microstl::Reader::readStlFile(file_path, handler);
}

std::string getInputFile(int argc, char **argv) {
  bool takeNext = false;
  for (int i = 0; i < argc; i++) {
    if (takeNext)
      return argv[i];

    if (strcmp(argv[i], "-i") == 0)
      takeNext = true;
  }
  return "";
}

std::string getOutputFile(int argc, char **argv) {
  bool takeNext = false;
  for (int i = 0; i < argc; i++) {
    if (takeNext)
      return argv[i];

    if (strcmp(argv[i], "-o") == 0)
      takeNext = true;
  }
  return "";
}

float getScaleFactor(int argc, char **argv) {
  bool takeNext = false;
  for (int i = 0; i < argc; i++) {
    if (takeNext)
      return std::stof(argv[i]);

    if (strcmp(argv[i], "-s") == 0)
      takeNext = true;
  }
  return 1.f;
}

bool isRequestingHelp(int argc, char **argv) {
  for (int i = 0; i < argc; i++) {
    if (strcmp(argv[i], "-h") == 0)
      return true;
  }
  return false;
}

int getMaxDepth(int argc, char **argv) {
  bool takeNext = false;
  for (int i = 0; i < argc; i++) {
    if (takeNext)
      return std::stoi(argv[i]);

    if (strcmp(argv[i], "-d") == 0)
      takeNext = true;
  }
  return -1;
}

void printHeader() {
  std::cout << " --- Kami, a paper pattern maker by Meltwin (2023) ---"
            << std::endl;
}

void printHelp() {
  printHeader();
  std::cout << "Usage : kami -i <input file> -o <output file>" << std::endl;
}

int main(int argc, char **argv) {
  // Get input / output files
  const std::string input_file = getInputFile(argc, argv);
  const std::string output_file = getOutputFile(argc, argv);
  const float scale_factor = getScaleFactor(argc, argv);
  const int max_depth = getMaxDepth(argc, argv);
  bool helpRequested = isRequestingHelp(argc, argv);

  if (helpRequested) {
    printHelp();
    return 0;
  }
  if (input_file.compare("") == 0) {
    printHelp();
    std::cout << "Error: No input file was given !" << std::endl;
    return -1;
  }
  if (output_file.compare("") == 0) {
    printHelp();
    std::cout << "Error: No output file was given !" << std::endl;
    return -1;
  }

  printHeader();
  std::cout << "Scale factor: " << scale_factor << std::endl;
  std::cout << "Max depth: " << max_depth << std::endl;

  // Load STL file
  microstl::MeshReaderHandler handler;
  handler.forceNormals = false;
  handler.disableNormals = false;
  std::cout << "Loading STL file" << std::endl;
  auto start_time = std::chrono::high_resolution_clock::now();
  if (microstl::Result result = loadSTL(handler, input_file);
      result != microstl::Result::Success) {
    std::cout << "Couldn't load file (" << input_file
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
  pool.unfold(max_depth);
  std::cout << pool << std::endl;
  pool.slice();

  // Extract pattern
  std::cout << "Making Facet Pattern" << std::endl;
  start_time = std::chrono::high_resolution_clock::now();
  std::ofstream file(output_file, std::ios::out | std::ios::trunc);
  if (file.is_open()) {
    file << pool.getAsSVGString(max_depth, scale_factor);
  }
  file.close();
  end_time = std::chrono::high_resolution_clock::now();
  std::cout << "\tTook "
            << (end_time - start_time) / std::chrono::milliseconds(1) << " ms"
            << std::endl;

  return 0;
}