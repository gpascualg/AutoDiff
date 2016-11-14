project(AutoDiff-Python)

RequireExternal(
    TARGET AutoDiff-Python
    EXCLUDE
    MODULE pybind/pybind11:master
    CONFIGURE_COMMAND "${CMAKE_COMMAND} -DPYTHON_EXECUTABLE=${PYTHON_EXECUTABLE} -DPYTHON_LIBRARY=${PYTHON_LIBRARY} -DPYTHON_INCLUDE_DIR=${PYTHON_INCLUDE_DIR} ."
    INC_PATH "include"
)

if (EXISTS "${CMAKE_BINARY_DIR}/third_party/src/pybind_pybind11/tools/")
    list(APPEND CMAKE_MODULE_PATH "${CMAKE_BINARY_DIR}/third_party/src/pybind_pybind11/tools/")

    # Find python and numpy
    set(Python_ADDITIONAL_VERSIONS 3.4 3.5 3.6 3.7)
    find_package(PythonLibsNew ${PYBIND11_PYTHON_VERSION} REQUIRED)
    #find_package(NumPy REQUIRED)

    AddToSources(
       TARGET AutoDiff-Python
       SRC_PATH ${CMAKE_CURRENT_SOURCE_DIR}
       GLOB_SEARCH ".hpp;.cpp"
    )


    # -[ Add dependencies
    set(INCLUDE_DIRS
        ${CMAKE_BINARY_DIR}/third_party/src/pybind_pybind11/include
        ${PYTHON_INCLUDE_DIRS}
    #    ${PYTHON_NUMPY_INCLUDE_DIR}
    )
    foreach (path ${INCLUDE_DIRS})
        AddToSources(
            TARGET AutoDiff-Python
            INC_PATH ${path}
        )
    endforeach()

    # AutoDiff dependency
    AddDependency(
        TARGET AutoDiff-Python
        DEPENDENCY AutoDiff-Lib
    )


    BuildNow(
       TARGET AutoDiff-Python
       SHARED_LIB
       NO_PREFIX
       OUTPUT_NAME autodiff
    )

    if (BUILD_PYTHON_TESTS)
        add_custom_target(BuildPythonDeps ALL DEPENDS pybind_pybind11)
    endif()
endif()
