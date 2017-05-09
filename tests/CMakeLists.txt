if (BUILD_TESTS)
    project(AutoDiff-Tests)

    RequireExternal(
        TARGET AutoDiff-Tests
        MODULE philsquared/Catch:master
        EXCLUDE
        SKIP_LINK
        INC_PATH "include"
    )

    set(SOURCE_FOLDERS
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/test_variable
        ${CMAKE_CURRENT_SOURCE_DIR}/test_tape
    )

    foreach (folder ${SOURCE_FOLDERS})
        file(GLOB TMP_TESTS ${folder}/*.test)

        foreach (test ${TMP_TESTS})
            set(TEST_TEMPLATE "float")
            configure_file(${test} ${test}_${TEST_TEMPLATE}.cpp)

            set(TEST_TEMPLATE "double")
            configure_file(${test} ${test}_${TEST_TEMPLATE}.cpp)
        endforeach()
    endforeach()


    AddAllToSources(
        TARGET AutoDiff-Tests
        DIRS ${SOURCE_FOLDERS}
        GLOB_SEARCH ".hpp;.cpp"
    )

    AddToIncludes(
        TARGET AutoDiff-Tests
        INC_PATH ${CMAKE_CURRENT_SOURCE_DIR}
    )

    AddDependency(
        TARGET AutoDiff-Tests
        DEPENDENCY AutoDiff-Lib
    )

    BuildNow(
        TARGET AutoDiff-Tests
        EXECUTABLE
        OUTPUT_NAME AutoDiff-Tests
    )

    if (UNIX)
        set_target_properties(AutoDiff-Lib PROPERTIES COMPILE_FLAGS "-g -O0 -fprofile-arcs -ftest-coverage")
        set_target_properties(AutoDiff-Tests PROPERTIES COMPILE_FLAGS "-g -O0 -fprofile-arcs -ftest-coverage")

        include(Coverage)
        SETUP_TARGET_FOR_COVERAGE(
            AutoDiff-Coverage
            AutoDiff-Tests
            coverage
        )
    endif()
endif()
