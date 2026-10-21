# Makes Catch2::Catch2WithMain available (issue #5).
#
# An installed Catch2 3.x is used when CMake can find one, so offline
# machines can build the tests. Otherwise the pinned release is downloaded,
# unless SNAKE_FETCH_CATCH2 is OFF.

set(SNAKE_CATCH2_VERSION "v3.8.1")

if(NOT TARGET Catch2::Catch2WithMain)
    find_package(Catch2 3 QUIET)
endif()

if(TARGET Catch2::Catch2WithMain)
    message(STATUS "SnakeXtreme: using installed Catch2 ${Catch2_VERSION}")
elseif(SNAKE_FETCH_CATCH2)
    message(STATUS "SnakeXtreme: fetching Catch2 ${SNAKE_CATCH2_VERSION}")
    include(FetchContent)
    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG ${SNAKE_CATCH2_VERSION}
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(Catch2)
    # Treat Catch2's headers as system headers, so SNAKE_WARNINGS_AS_ERRORS
    # judges project code only. (Installed packages get this by default.)
    get_target_property(snake_catch2_includes Catch2 INTERFACE_INCLUDE_DIRECTORIES)
    if(snake_catch2_includes)
        set_target_properties(Catch2 PROPERTIES
            INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${snake_catch2_includes}")
    endif()
else()
    message(FATAL_ERROR
        "Catch2 3.x was not found and SNAKE_FETCH_CATCH2 is OFF. Install "
        "Catch2 3, set Catch2_DIR, or configure with -DSNAKE_FETCH_CATCH2=ON.")
endif()
