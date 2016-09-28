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

add_subdirectory(src)
