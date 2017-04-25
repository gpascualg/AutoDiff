cmake_minimum_required(VERSION 2.8)
project(AutoDiff)

# -[ Add own includes
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake-utils)

# -[ Macros and functions
include(Configure)
include(Macros)
include(External)
ResetAllTargets()

# -[ VSCode CMake building
include(CMakeToolsHelpers OPTIONAL)


# -[ C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED on)


# -[ Options
option(BUILD_TESTS "Build tests" ON)
option(BUILD_DEBUG "Build with debug enabled" ON)
option(BUILD_STATIC_LIB "Build static libraries" ON)

if (BUILD_DEBUG)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Debug;Release;RelWithDebInfo" FORCE)
endif()

# -[ Start including
add_subdirectory(src)

# -[ Testing
add_subdirectory(tests)

# -[ Copy commands for autocompleters
CopyCommands()
