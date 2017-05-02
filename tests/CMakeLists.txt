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
    )

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

    include(Coverage)
    SETUP_TARGET_FOR_COVERAGE(
        AutoDiff-Coverage
        AutoDiff-Tests
        coverage
    )
endif()
