cmake_minimum_required(VERSION 3.12)
project(OverEditor CXX)

set(CMAKE_CXX_STANDARD 17)
# Load submodules
add_subdirectory(thirdparty)

set(OVEREDITOR_VERSION_MAJOR 0)
set(OVEREDITOR_VERSION_MINOR 1)
set(OVEREDITOR_VERSION_PATCH 0)

add_subdirectory(overeditor)