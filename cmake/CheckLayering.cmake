# Layering check, run as a CTest test (TC-MNT-01 / NFR-005).
#
#   cmake -DSNAKE_SOURCE_DIR=<repo> -P cmake/CheckLayering.cmake
#
# Starting from every source file of the SFML-free modules, follows their
# project includes ("snakextreme/...") and fails if any reached file includes
# an SFML header. Presentation code (src/presentation, src/main.cpp and the
# headers only it uses) is never visited, so it may use SFML freely.

cmake_minimum_required(VERSION 3.20)

if(NOT SNAKE_SOURCE_DIR)
    message(FATAL_ERROR "Pass -DSNAKE_SOURCE_DIR=<repository root>")
endif()

set(core_dirs app engine modes scoring gameplay persistence)
set(pending "")
# C8 power-ups (issue #14).
list(APPEND core_dirs powerups)
foreach(dir IN LISTS core_dirs)
    file(GLOB_RECURSE found
        "${SNAKE_SOURCE_DIR}/src/${dir}/*.cpp"
        "${SNAKE_SOURCE_DIR}/src/${dir}/*.hpp")
    list(APPEND pending ${found})
endforeach()

set(visited "")
set(violations "")

while(pending)
    list(POP_FRONT pending file)
    if(file IN_LIST visited)
        continue()
    endif()
    list(APPEND visited "${file}")

    file(STRINGS "${file}" include_lines REGEX "^[ \t]*#[ \t]*include")
    foreach(line IN LISTS include_lines)
        if(line MATCHES "[<\"]SFML/")
            file(RELATIVE_PATH shown "${SNAKE_SOURCE_DIR}" "${file}")
            list(APPEND violations "${shown}: ${line}")
        elseif(line MATCHES "\"(snakextreme/[^\"]+)\"")
            set(header "${SNAKE_SOURCE_DIR}/include/${CMAKE_MATCH_1}")
            if(EXISTS "${header}")
                list(APPEND pending "${header}")
            endif()
        endif()
    endforeach()
endwhile()

list(LENGTH visited checked)
if(violations)
    string(REPLACE ";" "\n  " report "${violations}")
    message(FATAL_ERROR
        "SFML must not be used by engine, modes, power-up, scoring, "
        "gameplay, persistence or app-state code:\n  ${report}")
endif()
message(STATUS "Layering OK: ${checked} SFML-free files checked")
