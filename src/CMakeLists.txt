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

#set(OPENBLAS_LIB NOTFOUND CACHE INTERNAL "" FORCE)
#set(OPENVML_LIB NOTFOUND CACHE INTERNAL "" FORCE)

# -[ Find some libraries
#SET(CMAKE_FIND_LIBRARY_SUFFIXES ".dll" ".lib" ".so" ".a")

find_library(OPENBLAS_LIB
    NAMES openblas libopenblas
    HINTS ${CMAKE_BINARY_DIR}/third_party/src/xianyi_OpenBLAS-build/lib
    NO_DEFAULT_PATH
)
find_library(OPENVML_LIB
    NAMES openvml
    HINTS ${CMAKE_BINARY_DIR}/third_party/src/xianyi_OpenVML-build/lib
    PATH_SUFFIXES Debug Release
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



if (WIN32)
    get_filename_component(OPENBLAS_PATH ${OPENBLAS_LIB} DIRECTORY)
    get_filename_component(OPENVML_PATH ${OPENVML_LIB} DIRECTORY)

    add_custom_target(CopyShared ALL
        ${CMAKE_COMMAND} -E copy "${OPENBLAS_PATH}/libopenblas.dll" "$<TARGET_FILE_DIR:AutoDiff>/libopenblas.dll"
        COMMAND ${CMAKE_COMMAND} -E copy "${OPENVML_PATH}/openvml.dll" "$<TARGET_FILE_DIR:AutoDiff>/openvml.dll"
        DEPENDS AutoDiff
        COMMENT "Copying OpenBLAS & OpenVML DLLs"
    )
endif()
