project(AutoDiff-Python)

RequireExternal(
    TARGET AutoDiff-Python
    EXCLUDE
    MODULE pybind/pybind11:master
    INC_PATH "include"
)

AddToSources(
   TARGET AutoDiff-Python
   SRC_PATH ${CMAKE_CURRENT_SOURCE_DIR}
   GLOB_SEARCH ".hpp;.cpp"
)

find_library(PYBIND11_LIB
    NAMES openvml
    HINTS ${CMAKE_BINARY_DIR}/third_party/src/pybind_pybind11-build/lib
    PATH_SUFFIXES Debug Release
    NO_DEFAULT_PATH
)

if (PYBIND11_LIB)
    # -[ Add dependencies
    AddDependency(
        TARGET AutoDiff-Python
        DEPENDENCY ${PYBIND11_LIB}
    )

    BuildNow(
       TARGET AutoDiff-Python
       SHARED_LIB
       OUTPUT_NAME AutoDiff
    )
endif()
