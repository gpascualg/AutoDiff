# -[ Find sources
AddToSources(
   TARGET AutoDiff
   SRC_PATH ${CMAKE_CURRENT_SOURCE_DIR}
   GLOB_SEARCH ".hpp;.cpp"
)


# -[ Add dependencies
set(OpenVML_PATCHES
    "git apply --whitespace=fix ${CMAKE_SOURCE_DIR}/patches/real_mul_div.patch"
    "git apply --whitespace=fix ${CMAKE_SOURCE_DIR}/patches/real_powx_acc.patch"
)
RequireExternal(
    TARGET AutoDiff
    EXCLUDE
    MODULE xianyi/OpenVML:master
    CONFIGURE_STEPS ${OpenVML_PATCHES}
    INC_PATH "include"
)
RequireExternal(
    TARGET AutoDiff
    EXCLUDE
    MODULE xianyi/OpenBLAS:master
    INC_PATH "/"
)


# -[ Find both libraries
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


# -[ Add dependencies
AddDependency(
    TARGET AutoDiff
    DEPENDENCY ${OPENBLAS_LIB}
)
AddDependency(
    TARGET AutoDiff
    DEPENDENCY ${OPENVML_LIB}
)


# -[ Should it use BLAS, add it
if (WITH_BLAS_SUPPORT)
    AddDefinition(
        TARGET AutoDiff
        DEFINITIONS "WITH_BLAS_SUPPORT"
    )
endif()


# -[ Build project
if (OPENVML_LIB AND OPENBLAS_LIB)
    BuildNow(
       TARGET AutoDiff
       EXECUTABLE
       OUTPUT_NAME AutoDiff
    )

    if (WITH_BLAS_SUPPORT AND WIN32)
        get_filename_component(OPENBLAS_PATH ${OPENBLAS_LIB} DIRECTORY)
        get_filename_component(OPENVML_PATH ${OPENVML_LIB} DIRECTORY)

        add_custom_target(CopyShared ALL
            ${CMAKE_COMMAND} -E copy "${OPENBLAS_PATH}/libopenblas.dll" "$<TARGET_FILE_DIR:AutoDiff>/libopenblas.dll"
            COMMAND ${CMAKE_COMMAND} -E copy "${OPENVML_PATH}/openvml.dll" "$<TARGET_FILE_DIR:AutoDiff>/openvml.dll"
            DEPENDS AutoDiff
            COMMENT "Copying OpenBLAS & OpenVML DLLs"
        )
    endif()
else()
    message("A first build will be done with dependencies only")
    add_custom_target(BuildDeps ALL DEPENDS xianyi_OpenVML xianyi_OpenBLAS)
    add_custom_target(Rebuild ALL
        ${CMAKE_COMMAND} ${CMAKE_SOURCE_DIR}
        COMMAND ${CMAKE_COMMAND} --build .
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        DEPENDS BuildDeps
    )
endif()
