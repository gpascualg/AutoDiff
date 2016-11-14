cmake_minimum_required(VERSION 2.8)
project(AutoDiff)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake-utils)


# -[ Macros and functions
include(Configure)
include(Macros)
ResetAllTargets()


# -[ C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED on)


# -[ Options
option(WITH_UNOPTIMIZED_CPU_SUPPORT "Build with unoptimized CPU support" ON)
option(WITH_BLAS_SUPPORT "Build with OpenBLAS and OpenVML" ON)
option(BUILD_DEBUG "Build with debug enabled" OFF)
option(BUILD_PYTHON "Build Python wrapper" ON)
option(BUILD_EMSCRIPTEN "Build Javascript port" OFF)

if (BUILD_DEBUG)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Debug;Release;RelWithDebInfo" FORCE)
endif()

# -[ Start including
add_subdirectory(src)

if (BUILD_PYTHON AND NOT BUILD_EMSCRIPTEN)
    add_subdirectory(python)
endif()
