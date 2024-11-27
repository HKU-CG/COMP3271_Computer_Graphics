cmake_minimum_required(VERSION 3.8)
project(Assignment3)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_BUILD_TYPE Release)

# glm
include_directories(Utils/glm)

file(GLOB CPP_FILES ./*.cpp)
file(GLOB HEADER_FILES ./*.h)


add_executable(Main Utils/lodepng.cpp ${CPP_FILES} ${HEADER_FILES})
target_compile_definitions(Main PRIVATE CMAKE_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/")
