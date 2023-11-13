cmake_minimum_required(VERSION 3.10)
project(kami_project)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(kami src/main.cpp src/meshTree.cpp src/svgfigure.cpp)
target_include_directories(kami PUBLIC include)