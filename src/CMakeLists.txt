# -[ Create target
CreateTarget(TARGET AutoDiff-Lib)

# -[ Build config file
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)

# -[ Include source directory
AddToIncludes(
    TARGET AutoDiff-Lib
    INC_PATH ${CMAKE_CURRENT_SOURCE_DIR}
)

# -[ Find sources
set(AUTODIFF_FOLDERS
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/helpers
    ${CMAKE_CURRENT_SOURCE_DIR}/tape
    ${CMAKE_CURRENT_SOURCE_DIR}/variable
)
AddAllToSources(
    TARGET AutoDiff-Lib
    DIRS ${AUTODIFF_FOLDERS}
    GLOB_SEARCH ".hpp;.cpp"
)

# -[ See if there is any unresolved external targets
ResolveExternal(TARGET AutoDiff-Lib)

# -[ Build project
if (AutoDiff-Lib_IS_RESOLVED)
    set(LIB_BUILD_TYPE "SHARED_LIB")
    if (BUILD_STATIC_LIB)
        set(LIB_BUILD_TYPE "STATIC_LIB")
    endif()

    if (BUILD_TESTS AND UNIX)
        AddDependency(
            TARGET AutoDiff-Lib
            DEPENDENCY gcov
        )
    endif()

    BuildNow(
       TARGET AutoDiff-Lib
       ${LIB_BUILD_TYPE}
       OUTPUT_NAME autodiff
    )

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
