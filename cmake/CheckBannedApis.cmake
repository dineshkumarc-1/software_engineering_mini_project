# Banned-API scan, run as a CTest test (SEC-005 / TC-SEC-05, issue #23).
#
#   cmake -DSNAKE_SOURCE_DIR=<repo> -P cmake/CheckBannedApis.cmake
#
# Product code (src/ and include/) must never run commands or load code
# (system, popen, exec*, spawn*, dlopen, LoadLibrary, ShellExecute,
# CreateProcess, WinExec), never use unbounded C string functions (strcpy,
# strcat, sprintf, vsprintf, gets), and never use the printf family at all,
# so no format string can come from a file. Data read from files is only
# ever data. Comments are ignored. A reviewed exception can be allowed on
# its own line with the marker "banned-api-ok: <reason>".

cmake_minimum_required(VERSION 3.20)

if(NOT SNAKE_SOURCE_DIR)
    message(FATAL_ERROR "Pass -DSNAKE_SOURCE_DIR=<repository root>")
endif()

set(banned_calls
    # Running commands and loading code.
    "system" "_?w?popen" "_?w?exec[lv]p?e?" "_?w?spawn[lv]p?e?" "posix_spawnp?"
    "dlopen" "LoadLibrary[A-Za-z]*" "ShellExecute[A-Za-z]*" "CreateProcess[A-Za-z]*"
    "WinExec"
    # Unbounded C string functions.
    "strcpy" "strcat" "wcscpy" "wcscat" "gets"
    # The whole printf family: printf, fprintf, sprintf, snprintf, vprintf,
    # wprintf, swprintf, _snprintf, ...
    "[A-Za-z_]*printf")
list(JOIN banned_calls "|" alternatives)
set(call_pattern "(^|[^A-Za-z0-9_])(${alternatives})[ \t]*\\(")

set(files "")
foreach(dir IN ITEMS src include)
    file(GLOB_RECURSE found
        "${SNAKE_SOURCE_DIR}/${dir}/*.cpp"
        "${SNAKE_SOURCE_DIR}/${dir}/*.hpp"
        "${SNAKE_SOURCE_DIR}/${dir}/*.h"
        "${SNAKE_SOURCE_DIR}/${dir}/*.inl")
    list(APPEND files ${found})
endforeach()
list(SORT files)

set(hits "")
foreach(file IN LISTS files)
    file(READ "${file}" content)
    # One list element per line (protect the semicolons CMake splits lists on).
    string(REPLACE ";" "<semicolon>" content "${content}")
    string(REPLACE "\n" ";" lines "${content}")
    set(number 0)
    set(in_comment FALSE)
    foreach(line IN LISTS lines)
        math(EXPR number "${number} + 1")
        if(line MATCHES "banned-api-ok:")
            continue()
        endif()

        # Blank out character and string literals first, so quotes, slashes
        # and call-like text inside them are never read as code.
        set(code "${line}")
        string(REGEX REPLACE "'([^'\\\\]|\\\\.)*'" "''" code "${code}")
        string(REGEX REPLACE "\"([^\"\\\\]|\\\\.)*\"" "\"\"" code "${code}")

        # Inside a block comment until its "*/".
        if(in_comment)
            string(FIND "${code}" "*/" close)
            if(close EQUAL -1)
                continue()
            endif()
            math(EXPR after_close "${close} + 2")
            string(SUBSTRING "${code}" ${after_close} -1 code)
            set(in_comment FALSE)
        endif()

        # Remove comments; whichever of // and /* comes first wins.
        while(TRUE)
            string(FIND "${code}" "//" line_comment)
            string(FIND "${code}" "/*" block_comment)
            if(block_comment EQUAL -1 AND line_comment EQUAL -1)
                break()
            endif()
            if(block_comment EQUAL -1 OR
               (NOT line_comment EQUAL -1 AND line_comment LESS block_comment))
                string(SUBSTRING "${code}" 0 ${line_comment} code)
                break()
            endif()
            string(SUBSTRING "${code}" 0 ${block_comment} before)
            math(EXPR after_open "${block_comment} + 2")
            string(SUBSTRING "${code}" ${after_open} -1 rest)
            string(FIND "${rest}" "*/" close)
            if(close EQUAL -1)
                set(code "${before}")
                set(in_comment TRUE)
                break()
            endif()
            math(EXPR after_close "${close} + 2")
            string(SUBSTRING "${rest}" ${after_close} -1 rest)
            set(code "${before} ${rest}")
        endwhile()

        if(code MATCHES "${call_pattern}")
            file(RELATIVE_PATH shown "${SNAKE_SOURCE_DIR}" "${file}")
            string(STRIP "${line}" trimmed)
            list(APPEND hits "${shown}:${number}: ${CMAKE_MATCH_2}() in: ${trimmed}")
        endif()
    endforeach()
endforeach()

list(LENGTH files checked)
if(hits)
    list(LENGTH hits count)
    string(REPLACE ";" "\n  " report "${hits}")
    string(REPLACE "<semicolon>" ";" report "${report}")
    message(FATAL_ERROR
        "${count} banned API call(s) (SEC-005): commands, code loading, "
        "unbounded C strings and the printf family are not allowed in "
        "src/ and include/:\n  ${report}")
endif()
message(STATUS "Banned-API scan OK: 0 hits in ${checked} files")
