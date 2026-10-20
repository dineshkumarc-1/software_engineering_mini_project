cmake_minimum_required(VERSION 3.20)
project(SnakeXtreme VERSION 0.1.0 LANGUAGES CXX)

option(SNAKE_BUILD_APP "Build the SFML desktop application" ON)
option(SNAKE_FETCH_SFML "Download pinned SFML 2.5.1 instead of finding it" OFF)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

function(snake_warnings target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4 /permissive-)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endfunction()

# No SFML dependency: state flow can be tested on machines without a display.
add_library(snake_app_state src/app/AppState.cpp)
target_include_directories(snake_app_state PUBLIC include)
snake_warnings(snake_app_state)
# SFML-free core gameplay engine, suitable for headless testing.
add_library(snake_engine src/engine/GameEngine.cpp)
target_include_directories(snake_engine PUBLIC include)
snake_warnings(snake_engine)

add_library(snake_scoring src/scoring/ScoreService.cpp)
target_include_directories(snake_scoring PUBLIC include)
snake_warnings(snake_scoring)

if(SNAKE_BUILD_APP)
    if(SNAKE_FETCH_SFML)
        include(FetchContent)
        set(SFML_BUILD_AUDIO OFF CACHE BOOL "" FORCE)
        set(SFML_BUILD_NETWORK OFF CACHE BOOL "" FORCE)
        set(SFML_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
        set(SFML_BUILD_DOC OFF CACHE BOOL "" FORCE)
        FetchContent_Declare(sfml
            GIT_REPOSITORY https://github.com/SFML/SFML.git
            GIT_TAG 2.5.1
            GIT_SHALLOW TRUE)
        FetchContent_MakeAvailable(sfml)
    else()
        find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)
        if(DEFINED SFML_VERSION AND NOT SFML_VERSION VERSION_LESS 3)
            message(FATAL_ERROR "SnakeXtreme requires SFML 2.x, not SFML 3")
        endif()
    endif()
    add_library(snake_presentation
        src/presentation/AppManager.cpp
        src/presentation/Renderer.cpp)
    target_include_directories(snake_presentation PUBLIC include)
    target_link_libraries(snake_presentation PUBLIC snake_app_state snake_engine sfml-graphics sfml-window sfml-system)
    snake_warnings(snake_presentation)
    add_executable(SnakeXtreme src/main.cpp)
    target_link_libraries(
        SnakeXtreme
        PRIVATE
        snake_presentation
        snake_scoring
    )
    snake_warnings(SnakeXtreme)
    # Shared-library builds on Windows need the SFML DLLs beside the executable.
    if(WIN32 AND SNAKE_FETCH_SFML AND BUILD_SHARED_LIBS)
        foreach(module graphics window system)
            add_custom_command(TARGET SnakeXtreme POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:sfml-${module}> $<TARGET_FILE_DIR:SnakeXtreme>)
        endforeach()
    endif()
endif()

include(CTest)
if(BUILD_TESTING)
    add_executable(app_state_tests tests/AppStateTests.cpp)
    target_link_libraries(app_state_tests PRIVATE snake_app_state)
    snake_warnings(app_state_tests)
    add_test(NAME app_state_flow COMMAND app_state_tests)


    add_executable(score_service_tests tests/ScoreServiceTests.cpp)
    target_link_libraries(score_service_tests PRIVATE snake_scoring)
    snake_warnings(score_service_tests)
    add_test(NAME score_service COMMAND score_service_tests)

    add_executable(game_session_tests tests/GameSessionTests.cpp)
    target_link_libraries(game_session_tests PRIVATE snake_scoring)
    snake_warnings(game_session_tests)
    add_test(NAME game_session COMMAND game_session_tests)

    add_executable(game_engine_tests tests/GameEngineTests.cpp)
    target_link_libraries(game_engine_tests PRIVATE snake_engine)
    snake_warnings(game_engine_tests)
    add_test(NAME game_engine COMMAND game_engine_tests)
endif()

