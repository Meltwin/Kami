cmake_minimum_required(VERSION 3.10)
project(kami_project)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB S_FILES src/*.cpp)

find_package( Eigen3 REQUIRED )

add_executable(kami "${S_FILES}")
target_include_directories(kami PUBLIC include EIGEN3_INCLUDE_DIR)