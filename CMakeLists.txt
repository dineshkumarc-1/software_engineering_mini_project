cmake_minimum_required(VERSION 3.20)
project(SnakeXtreme VERSION 0.1.0 LANGUAGES CXX)

option(SNAKE_BUILD_APP "Build the SFML desktop application" ON)
option(SNAKE_FETCH_SFML "Download pinned SFML 2.5.1 instead of finding it" OFF)
option(SNAKE_FETCH_CATCH2 "Download pinned Catch2 v3 when no installed Catch2 3.x is found" ON)
option(SNAKE_WARNINGS_AS_ERRORS "Treat compiler warnings in project code as errors" OFF)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")
include(SnakeModules)

# ---------------------------------------------------------------------------
# SFML-free modules (SAD layered architecture, NFR-005 / NFR-007).
# Each builds and tests on a machine without SFML or a display.
# ---------------------------------------------------------------------------

add_library(snake_scoring src/scoring/ScoreService.cpp)
target_include_directories(snake_scoring PUBLIC include)
snake_warnings(snake_scoring)
# C10 input validation (SEC-001, SEC-006).
snake_add_module(snake_validation
    SOURCES src/validation/InputValidator.cpp)

# C1 navigation model (screen state flow).
snake_add_module(snake_app_state SOURCES src/app/AppState.cpp)
# C6 core gameplay engine.
snake_add_module(snake_engine SOURCES src/engine/GameEngine.cpp)
# C7 game modes and difficulty profiles. Uses engine grid types only.
snake_add_module(snake_modes
    SOURCES src/modes/ModeStrategy.cpp src/modes/ModeManager.cpp)
# One game as the app runs it: engine + ModeManager + score session.
snake_add_module(snake_gameplay
    SOURCES src/gameplay/GameplayController.cpp
    DEPS snake_app_state snake_engine snake_modes snake_scoring)
# C11-C13 persistence. Placeholder until issue #15 adds sources, e.g.
#   snake_add_module(snake_persistence SOURCES src/persistence/SecureFileStore.cpp)
snake_add_module(snake_persistence DEPS snake_validation)

# C12 settings schema/editor and interim atomic file adapter (#20).
include(FetchContent)
set(JSON_BuildTests OFF CACHE BOOL "" FORCE)
FetchContent_Declare(nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
    GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(nlohmann_json)
snake_add_module(snake_settings
    SOURCES src/persistence/Settings.cpp src/persistence/SettingsRepository.cpp)
target_link_libraries(snake_settings PRIVATE nlohmann_json::nlohmann_json)

# Optional C1 performance diagnostics; no SFML or player data in reports.
snake_add_module(snake_probe SOURCES src/diagnostics/PerformanceProbe.cpp)
target_link_libraries(snake_probe PRIVATE nlohmann_json::nlohmann_json)

# C5 waveform generation and volume policy remain SFML-free.
snake_add_module(snake_audio_model SOURCES src/audio/AudioModel.cpp)
snake_assert_core_is_sfml_free()

# ---------------------------------------------------------------------------
# Presentation layer (C1-C5) and the desktop executable.
# ---------------------------------------------------------------------------
if(SNAKE_BUILD_APP)
    if(SNAKE_FETCH_SFML)
        include(FetchContent)
        set(SFML_BUILD_AUDIO ON CACHE BOOL "" FORCE)
        set(SFML_BUILD_NETWORK OFF CACHE BOOL "" FORCE)
        set(SFML_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
        set(SFML_BUILD_DOC OFF CACHE BOOL "" FORCE)
        FetchContent_Declare(sfml
            GIT_REPOSITORY https://github.com/SFML/SFML.git
            GIT_TAG 2.5.1
            GIT_SHALLOW TRUE)
        # SFML 2.5.1 asks for CMake 3.0.2; CMake 4 refuses anything below 3.5
        # unless this is set. Older CMake versions ignore the variable.
        set(snake_saved_policy_minimum "${CMAKE_POLICY_VERSION_MINIMUM}")
        if(NOT CMAKE_POLICY_VERSION_MINIMUM)
            set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
        endif()
        FetchContent_MakeAvailable(sfml)
        # SFML 2.5.1 AudioDevice.cpp uses std::auto_ptr. Compile this legacy
        # dependency target as C++14 (where it exists), not the project's C++17.
        # No app/core standard or warning flags are weakened.
        set_target_properties(sfml-audio PROPERTIES
            CXX_STANDARD 14 CXX_STANDARD_REQUIRED YES)
        set(CMAKE_POLICY_VERSION_MINIMUM "${snake_saved_policy_minimum}")
        # Treat SFML's headers as system headers (as an installed SFML is),
        # so project warnings and SNAKE_WARNINGS_AS_ERRORS judge project
        # code only, not SFML's own inline code.
        foreach(module system window graphics audio)
            get_target_property(snake_sfml_includes sfml-${module} INTERFACE_INCLUDE_DIRECTORIES)
            if(snake_sfml_includes)
                set_target_properties(sfml-${module} PROPERTIES
                    INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${snake_sfml_includes}")
            endif()
        endforeach()
    else()
        find_package(SFML 2.5 COMPONENTS graphics window system audio REQUIRED)
        if(DEFINED SFML_VERSION AND NOT SFML_VERSION VERSION_LESS 3)
            message(FATAL_ERROR "SnakeXtreme requires SFML 2.x, not SFML 3")
        endif()
    endif()
    add_library(snake_presentation
        src/presentation/AppManager.cpp
        src/presentation/Renderer.cpp
        src/presentation/AudioService.cpp)
    target_include_directories(snake_presentation PUBLIC include)
    target_compile_definitions(snake_presentation PRIVATE
        SNAKE_ASSET_DIR="${CMAKE_CURRENT_SOURCE_DIR}/assets")
   target_link_libraries(snake_presentation PUBLIC
    snake_app_state snake_engine snake_modes snake_scoring snake_gameplay
    snake_persistence snake_validation snake_settings snake_audio_model snake_probe
    sfml-graphics sfml-window sfml-system sfml-audio)
    snake_warnings(snake_presentation)
    add_executable(SnakeXtreme src/main.cpp)
    add_custom_command(TARGET SnakeXtreme POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory
            "$<TARGET_FILE_DIR:SnakeXtreme>/assets/fonts"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/assets/fonts/dejavu-sans-ttf-2.37/ttf/DejaVuSans.ttf"
            "$<TARGET_FILE_DIR:SnakeXtreme>/assets/fonts/DejaVuSans.ttf"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/assets/licenses"
            "$<TARGET_FILE_DIR:SnakeXtreme>/assets/licenses"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/THIRD_PARTY_NOTICES.md"
            "$<TARGET_FILE_DIR:SnakeXtreme>/THIRD_PARTY_NOTICES.md"
    )
    target_link_libraries(
        SnakeXtreme
        PRIVATE
        snake_presentation
        snake_scoring
    )
    snake_warnings(SnakeXtreme)
    # Shared-library builds on Windows need the SFML DLLs beside the executable.
    if(WIN32 AND SNAKE_FETCH_SFML AND BUILD_SHARED_LIBS)
        foreach(module graphics window system audio)
            add_custom_command(TARGET SnakeXtreme POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:sfml-${module}> $<TARGET_FILE_DIR:SnakeXtreme>)
        endforeach()
    endif()
    if(WIN32 AND SNAKE_FETCH_SFML)
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            set(SNAKE_OPENAL_ARCH x64)
        else()
            set(SNAKE_OPENAL_ARCH x86)
        endif()
        # SFML 2.5.1 ships the matching OpenAL runtime with its Windows sources.
        add_custom_command(TARGET SnakeXtreme POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${sfml_SOURCE_DIR}/extlibs/bin/${SNAKE_OPENAL_ARCH}/openal32.dll"
            "$<TARGET_FILE_DIR:SnakeXtreme>/openal32.dll")
    endif()
endif()

# ---------------------------------------------------------------------------
# Tests: headless, no SFML or display needed (issue #5, TC-TEST-01).
# Run with `ctest --test-dir <build> -C Release --output-on-failure` or
# `cmake --build <build> --config Release --target check`.
# ---------------------------------------------------------------------------
include(CTest)
if(BUILD_TESTING)
    include(SnakeCatch2)

    # Small dependency-free harness kept from issue #4.
    add_executable(app_state_tests tests/AppStateTests.cpp)
    target_link_libraries(app_state_tests PRIVATE snake_app_state snake_validation)
    snake_warnings(app_state_tests)
    add_test(NAME app_state_flow COMMAND app_state_tests)


    add_executable(score_service_tests tests/ScoreServiceTests.cpp)
    target_link_libraries(score_service_tests PRIVATE snake_scoring)
    snake_warnings(score_service_tests)
    add_test(NAME score_service COMMAND score_service_tests)

    # C10 input validator tests (SEC-001, SEC-006).
snake_add_catch2_test(input_validator_tests
    SOURCES tests/InputValidatorTests.cpp
    LIBS snake_validation)

    add_executable(game_session_tests tests/GameSessionTests.cpp)
    target_link_libraries(game_session_tests PRIVATE snake_scoring snake_engine)
    snake_warnings(game_session_tests)
    add_test(NAME game_session COMMAND game_session_tests)
    snake_add_catch2_test(game_engine_tests
        SOURCES tests/GameEngineTests.cpp
        LIBS snake_engine)

    snake_add_catch2_test(game_mode_tests
        SOURCES tests/GameModeTests.cpp tests/ModeEngineIntegrationTests.cpp
        LIBS snake_modes snake_engine)

    # Menu selection and the app's game wiring (modes, speed, timer, score).
    snake_add_catch2_test(gameplay_tests
        SOURCES tests/GameplayControllerTests.cpp
        LIBS snake_gameplay)

    snake_add_catch2_test(settings_tests
        SOURCES tests/SettingsTests.cpp
        LIBS snake_settings nlohmann_json::nlohmann_json)

    snake_add_catch2_test(audio_model_tests
        SOURCES tests/AudioModelTests.cpp
        LIBS snake_audio_model snake_settings)

    snake_add_catch2_test(performance_probe_tests
        SOURCES tests/PerformanceProbeTests.cpp
        LIBS snake_probe snake_gameplay nlohmann_json::nlohmann_json)

    # Optional real SFML/OpenAL smoke executable; not a device-dependent CTest.
    if(SNAKE_BUILD_APP)
        add_executable(audio_backend_smoke tests/AudioBackendSmoke.cpp)
        target_link_libraries(audio_backend_smoke PRIVATE snake_presentation)
        snake_warnings(audio_backend_smoke)
    endif()

    # TC-MNT-01: SFML-free code must not include SFML headers.
    add_test(NAME layering_core_is_sfml_free
        COMMAND ${CMAKE_COMMAND} -DSNAKE_SOURCE_DIR=${PROJECT_SOURCE_DIR}
                -P ${PROJECT_SOURCE_DIR}/cmake/CheckLayering.cmake)

    # Test runner: builds every test executable, then runs the whole suite.
    get_property(snake_test_targets GLOBAL PROPERTY SNAKE_TEST_TARGETS)
    add_custom_target(check
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
                -C $<IF:$<BOOL:$<CONFIG>>,$<CONFIG>,Debug>
        WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
        DEPENDS app_state_tests ${snake_test_targets}
        USES_TERMINAL
        VERBATIM
        COMMENT "Running SnakeXtreme tests")
endif()
