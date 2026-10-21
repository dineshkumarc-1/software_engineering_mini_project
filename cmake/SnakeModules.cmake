# Helpers shared by every SnakeXtreme target (issue #5).

# Warning flags for one target. Kept under this name because other branches
# already call snake_warnings(<target>).
function(snake_warnings target)
    get_target_property(type ${target} TYPE)
    if(type STREQUAL "INTERFACE_LIBRARY")
        return()
    endif()
    if(MSVC)
        # /utf-8: read sources as UTF-8 regardless of the Windows code page.
        target_compile_options(${target} PRIVATE /W4 /permissive- /utf-8)
        if(SNAKE_WARNINGS_AS_ERRORS)
            target_compile_options(${target} PRIVATE /WX)
        endif()
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
        if(SNAKE_WARNINGS_AS_ERRORS)
            target_compile_options(${target} PRIVATE -Werror)
        endif()
    endif()
endfunction()

# snake_add_module(<name> [SOURCES <files>...] [DEPS <targets>...])
#
# Adds one SFML-free logic/persistence module as a static library with the
# shared include directory and warnings. A module with no sources yet becomes
# an INTERFACE placeholder, so other targets can already link it; adding
# SOURCES later turns it into a real library without touching its users.
function(snake_add_module name)
    cmake_parse_arguments(ARG "" "" "SOURCES;DEPS" ${ARGN})

    if(ARG_SOURCES)
        add_library(${name} STATIC ${ARG_SOURCES})
        target_include_directories(${name} PUBLIC ${PROJECT_SOURCE_DIR}/include)
        if(ARG_DEPS)
            target_link_libraries(${name} PUBLIC ${ARG_DEPS})
        endif()
        snake_warnings(${name})
    else()
        add_library(${name} INTERFACE)
        target_include_directories(${name} INTERFACE ${PROJECT_SOURCE_DIR}/include)
        if(ARG_DEPS)
            target_link_libraries(${name} INTERFACE ${ARG_DEPS})
        endif()
    endif()

    set_property(GLOBAL APPEND PROPERTY SNAKE_CORE_MODULES ${name})
endfunction()

# Fails configuration if any SFML-free module links an SFML target
# (NFR-005 / NFR-007: logic and persistence must build without SFML).
function(snake_assert_core_is_sfml_free)
    get_property(modules GLOBAL PROPERTY SNAKE_CORE_MODULES)
    foreach(module IN LISTS modules)
        get_target_property(type ${module} TYPE)
        if(type STREQUAL "INTERFACE_LIBRARY")
            get_target_property(links ${module} INTERFACE_LINK_LIBRARIES)
        else()
            get_target_property(links ${module} LINK_LIBRARIES)
        endif()
        if(links)
            foreach(link IN LISTS links)
                if(link MATCHES "sfml")
                    message(FATAL_ERROR
                        "${module} must stay SFML-free but links ${link}")
                endif()
            endforeach()
        endif()
    endforeach()
endfunction()

# snake_add_catch2_test(<name> SOURCES <files>... LIBS <targets>...)
#
# One Catch2 v3 test executable registered with CTest. Run every test with
# `ctest` or `cmake --build <dir> --target check`.
function(snake_add_catch2_test name)
    cmake_parse_arguments(ARG "" "" "SOURCES;LIBS" ${ARGN})
    add_executable(${name} ${ARG_SOURCES})
    target_link_libraries(${name} PRIVATE ${ARG_LIBS} Catch2::Catch2WithMain)
    snake_warnings(${name})
    add_test(NAME ${name} COMMAND ${name})
    set_property(GLOBAL APPEND PROPERTY SNAKE_TEST_TARGETS ${name})
endfunction()
