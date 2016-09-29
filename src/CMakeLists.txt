# -[ Find sources
AddToSources(
   TARGET AutoDiff
   SRC_PATH ${CMAKE_CURRENT_SOURCE_DIR}
   GLOB_SEARCH ".hpp;.cpp"
)

# -[ Add dependencies
RequireExternal(
    TARGET AutoDiff
    EXCLUDE
    MODULE xianyi/OpenBLAS:master
    INC_PATH "/"
)
RequireExternal(
    TARGET AutoDiff
    EXCLUDE
    MODULE xianyi/OpenVML:master
    INC_PATH "include"
)


# -[ Find some libraries
find_library(OPENBLAS_LIB
    NAMES openblas libopenblas
    HINTS ${CMAKE_BINARY_DIR}/third_party/src/xianyi_OpenBLAS-build/lib
    NO_DEFAULT_PATH
)
find_library(OPENVML_LIB
    NAMES openvml
    HINTS ${CMAKE_BINARY_DIR}/third_party/src/xianyi_OpenVML-build/lib
    NO_DEFAULT_PATH
)

AddDependency(
    TARGET AutoDiff
    DEPENDENCY ${OPENBLAS_LIB}
)
AddDependency(
    TARGET AutoDiff
    DEPENDENCY ${OPENVML_LIB}
)

if (OPENVML_LIB)
    # -[ Build project
    BuildNow(
       TARGET AutoDiff
       EXECUTABLE
       OUTPUT_NAME AutoDiff
    )
else()
    message("Make once first to build all dependencies and rerun cmake")
    add_custom_target(BuildDeps ALL DEPENDS xianyi_OpenVML xianyi_OpenBLAS)
endif()
