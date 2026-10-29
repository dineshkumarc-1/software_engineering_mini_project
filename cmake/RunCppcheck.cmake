# cppcheck gate (issue #23, NFR-005): all project C++ code, any finding fails.
#
#   cmake -P cmake/RunCppcheck.cmake
#
# Needs cppcheck on the PATH (CI uses Ubuntu 24.04's cppcheck 2.13). Checks
# src/, include/ and tests/ with the warning, style, performance and
# portability checks. A justified false positive is suppressed on its own
# line with "// cppcheck-suppress <id>" and a comment saying why.

cmake_minimum_required(VERSION 3.20)

if(NOT SNAKE_SOURCE_DIR)
    get_filename_component(SNAKE_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)
endif()

find_program(CPPCHECK_EXECUTABLE cppcheck)
if(NOT CPPCHECK_EXECUTABLE)
    message(FATAL_ERROR "cppcheck was not found on the PATH")
endif()
execute_process(COMMAND "${CPPCHECK_EXECUTABLE}" --version
    OUTPUT_VARIABLE version OUTPUT_STRIP_TRAILING_WHITESPACE)
message(STATUS "Running ${version}")

execute_process(
    COMMAND "${CPPCHECK_EXECUTABLE}"
        --std=c++17
        --language=c++
        --enable=warning,style,performance,portability
        --inline-suppr
        # System, SFML and Catch2 headers are not analysed.
        --suppress=missingIncludeSystem
        --error-exitcode=1
        --quiet
        "--template={file}:{line}: {severity}: {message} [{id}]"
        -I include
        # Deliberately bad code that tests the banned-API scan.
        -i tests/fixtures
        src include tests
    WORKING_DIRECTORY "${SNAKE_SOURCE_DIR}"
    RESULT_VARIABLE result)

if(NOT result EQUAL 0)
    message(FATAL_ERROR "cppcheck reported findings (listed above)")
endif()
message(STATUS "cppcheck: no findings in src/, include/ and tests/")
