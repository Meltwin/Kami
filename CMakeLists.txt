cmake_minimum_required(VERSION 3.10)
project(kami)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(-g -O3)

# =============================================================================
# Libraries
# =============================================================================
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

set(OpenGL_GL_PREFERENCE "GLVND")
find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIRS})

find_package(Eigen3 REQUIRED)
include_directories(${EIGEN3_INCLUDE_DIR})

include_directories("libs/imgui")
file(GLOB IMGUI_SRC "libs/imgui/**.cpp")
set(IMGUI_SRC
        ${IMGUI_SRC}
        "libs/imgui/backends/imgui_impl_sdl2.cpp"
        "libs/imgui/backends/imgui_impl_opengl2.cpp"
)

include_directories("${CMAKE_CURRENT_BINARY_DIR}/im2cpp/")

# =============================================================================
# App
# =============================================================================
file(GLOB_RECURSE S_FILES src/*.cpp)
add_executable(${PROJECT_NAME} "${S_FILES}" ${IMGUI_SRC})
target_include_directories(${PROJECT_NAME} PUBLIC include)
target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARIES} ${OPENGL_LIBRARIES})