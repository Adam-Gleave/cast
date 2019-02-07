# CMakeList.txt : CMake project for cast, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

set(SDL2_INCLUDE_DIRS "${CMAKE_CURRENT_LIST_DIR}/include")

include_directories(${SDL2_INCLUDE_DIRS})

# TODO: Add tests and install targets if needed.
