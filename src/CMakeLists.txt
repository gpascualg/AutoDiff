AddToSources(
   TARGET AutoDiff
   SRC_PATH ${CMAKE_CURRENT_SOURCE_DIR}
   GLOB_SEARCH ".hpp;.cpp"
)

RequireExternal(
    TARGET AutoDiff
    EXCLUDE ON
    MODULE xianyi/OpenBLAS:master
    INC_PATH "/"
)

find_library(OPENBLAS_LIB
    NAMES openblas libopenblas
    HINTS ${CMAKE_BINARY_DIR}/third_party/src/xianyi_OpenBLAS-build/lib
    NO_DEFAULT_PATH
)

AddDependency(
    TARGET AutoDiff
    DEPENDENCY ${OPENBLAS_LIB}
)

BuildNow(
   TARGET AutoDiff
   EXECUTABLE ON
   OUTPUT_NAME AutoDiff
)
