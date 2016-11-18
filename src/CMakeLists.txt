# -[ Create target
project(AutoDiff-Lib)
CreateTarget(TARGET AutoDiff-Lib)

# -[ Build config file
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)


# -[ Find sources
set(AUTODIFF_FOLDERS
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/optimizer
    ${CMAKE_CURRENT_SOURCE_DIR}/pool
    ${CMAKE_CURRENT_SOURCE_DIR}/tape
    ${CMAKE_CURRENT_SOURCE_DIR}/variable
)
foreach (folder ${AUTODIFF_FOLDERS})
    AddToSources(
       TARGET AutoDiff-Lib
       SRC_PATH ${folder}
       GLOB_SEARCH ".hpp;.cpp"
    )
endforeach()


# -[ Add dependencies
if (WITH_BLAS_SUPPORT)
    set(OpenVML_PATCHES
        "git apply --whitespace=fix ${CMAKE_SOURCE_DIR}/patches/real_mul_div.patch"
        "git apply --whitespace=fix ${CMAKE_SOURCE_DIR}/patches/real_powx_acc.patch"
    )
    RequireExternal(
        TARGET AutoDiff-Lib
        MODULE xianyi/OpenVML:master
        EXCLUDE
        SKIP_LINK
        CONFIGURE_STEPS ${OpenVML_PATCHES}
        INC_PATH "include"
    )
    RequireExternal(
        TARGET AutoDiff-Lib
        MODULE xianyi/OpenBLAS:master
        EXCLUDE
        SKIP_LINK
        INC_PATH "/"
    )
endif()

ResolveExternal(TARGET AutoDiff-Lib)

# -[ Build project
if (AutoDiff-Lib_IS_RESOLVED)
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
        TARGET AutoDiff-Lib
        DEPENDENCY ${OPENBLAS_LIB}
    )
    AddDependency(
        TARGET AutoDiff-Lib
        DEPENDENCY ${OPENVML_LIB}
    )

    set(LIB_BUILD_TYPE "SHARED_LIB")
    if (BUILD_STATIC_LIB)
        set(LIB_BUILD_TYPE "STATIC_LIB")
    endif()

    BuildNow(
       TARGET AutoDiff-Lib
       ${LIB_BUILD_TYPE}
       OUTPUT_NAME autodiff
    )

    if (NOT BUILD_EMSCRIPTEN)
        AddToSources(
           TARGET AutoDiff-Exe
           SRC_PATH ${CMAKE_CURRENT_SOURCE_DIR}/exe
           GLOB_SEARCH ".hpp;.cpp"
        )

        AddDependency(
            TARGET AutoDiff-Exe
            DEPENDENCY AutoDiff-Lib
        )

        BuildNow(
           TARGET AutoDiff-Exe
           EXECUTABLE
           OUTPUT_NAME autodiff
        )
    endif()

    if (WITH_BLAS_SUPPORT AND WIN32)
        get_filename_component(OPENBLAS_PATH ${OPENBLAS_LIB} DIRECTORY)
        get_filename_component(OPENVML_PATH ${OPENVML_LIB} DIRECTORY)

        add_custom_target(CopyShared ALL
            ${CMAKE_COMMAND} -E copy "${OPENBLAS_PATH}/libopenblas.dll" "$<TARGET_FILE_DIR:AutoDiff-Lib>/libopenblas.dll"
            COMMAND ${CMAKE_COMMAND} -E copy "${OPENVML_PATH}/openvml.dll" "$<TARGET_FILE_DIR:AutoDiff-Lib>/openvml.dll"
            DEPENDS AutoDiff-Lib
            COMMENT "Copying OpenBLAS & OpenVML DLLs"
        )
    endif()
endif()
