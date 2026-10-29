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
# JSON dependency required by persistence.
find_package(nlohmann_json 3.11 QUIET)

if(NOT TARGET nlohmann_json::nlohmann_json)
    include(FetchContent)

    FetchContent_Declare(
        nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.11.3
        GIT_SHALLOW TRUE
    )

    FetchContent_MakeAvailable(nlohmann_json)
endif()

# C11-C13 persistence.
snake_add_module(snake_persistence
    SOURCES
        src/persistence/SecureFileStore.cpp
        src/persistence/LeaderboardRepository.cpp
    DEPS
        snake_validation
        nlohmann_json::nlohmann_json
)

snake_assert_core_is_sfml_free()

# ---------------------------------------------------------------------------
# Presentation layer (C1-C5) and the desktop executable.
# ---------------------------------------------------------------------------
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
        # SFML 2.5.1 asks for CMake 3.0.2; CMake 4 refuses anything below 3.5
        # unless this is set. Older CMake versions ignore the variable.
        set(snake_saved_policy_minimum "${CMAKE_POLICY_VERSION_MINIMUM}")
        if(NOT CMAKE_POLICY_VERSION_MINIMUM)
            set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
        endif()
        FetchContent_MakeAvailable(sfml)
        set(CMAKE_POLICY_VERSION_MINIMUM "${snake_saved_policy_minimum}")
        # Treat SFML's headers as system headers (as an installed SFML is),
        # so project warnings and SNAKE_WARNINGS_AS_ERRORS judge project
        # code only, not SFML's own inline code.
        foreach(module system window graphics)
            get_target_property(snake_sfml_includes sfml-${module} INTERFACE_INCLUDE_DIRECTORIES)
            if(snake_sfml_includes)
                set_target_properties(sfml-${module} PROPERTIES
                    INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${snake_sfml_includes}")
            endif()
        endforeach()
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
    target_compile_definitions(snake_presentation PRIVATE
        SNAKE_ASSET_DIR="${CMAKE_CURRENT_SOURCE_DIR}/assets")
   target_link_libraries(snake_presentation PUBLIC
    snake_app_state snake_engine snake_modes snake_scoring snake_gameplay
    snake_persistence snake_validation
    sfml-graphics sfml-window sfml-system)
    snake_warnings(snake_presentation)
    add_executable(SnakeXtreme src/main.cpp)
    add_custom_command(TARGET SnakeXtreme POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory
            "$<TARGET_FILE_DIR:SnakeXtreme>/assets/fonts"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/assets/fonts/dejavu-sans-ttf-2.37/ttf/DejaVuSans.ttf"
            "$<TARGET_FILE_DIR:SnakeXtreme>/assets/fonts/DejaVuSans.ttf"
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
        foreach(module graphics window system)
            add_custom_command(TARGET SnakeXtreme POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:sfml-${module}> $<TARGET_FILE_DIR:SnakeXtreme>)
        endforeach()
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
    target_link_libraries(app_state_tests PRIVATE snake_app_state)
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
        snake_add_catch2_test(secure_file_store_tests
        SOURCES tests/SecureFileStoreTests.cpp
        LIBS snake_persistence
    )

    snake_add_catch2_test(leaderboard_repository_tests
        SOURCES tests/LeaderboardRepositoryTests.cpp
        LIBS snake_persistence
    )

    # Issue #18: leaderboard and security test execution with the JSON
    # corpus (25 valid + 25 malformed files per file type, 64 KiB boundary).
    snake_add_catch2_test(security_corpus_tests
        SOURCES tests/SecurityCorpusTests.cpp
        LIBS snake_persistence)
    target_compile_definitions(security_corpus_tests PRIVATE
        SNAKE_JSON_CORPUS_DIR="${PROJECT_SOURCE_DIR}/tests/fixtures/json-corpus")
    # The config.json half, built once the settings module (C12, #20) exists.
    if(TARGET snake_settings)
        snake_add_catch2_test(settings_corpus_tests
            SOURCES tests/SettingsCorpusTests.cpp
            LIBS snake_settings nlohmann_json::nlohmann_json)
        target_compile_definitions(settings_corpus_tests PRIVATE
            SNAKE_JSON_CORPUS_DIR="${PROJECT_SOURCE_DIR}/tests/fixtures/json-corpus")
    endif()

    # TC-SEC-04 with real fault injection (Linux): an LD_PRELOAD library aborts
    # or fails the save at each step of write-temp-then-rename, in a child
    # process that performs one save.
    if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        add_library(snake_fault_inject SHARED tests/fault/FaultInjection.cpp)
        target_link_libraries(snake_fault_inject PRIVATE ${CMAKE_DL_LIBS})
        # It replaces libc functions: no fortified wrappers, no sanitizers.
        target_compile_options(snake_fault_inject PRIVATE
            -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=0 -fno-sanitize=all)
        target_link_options(snake_fault_inject PRIVATE -fno-sanitize=all)
        snake_warnings(snake_fault_inject)

        add_executable(secure_write_fault_writer tests/fault/FaultWriter.cpp)
        target_link_libraries(secure_write_fault_writer PRIVATE snake_persistence)
        snake_warnings(secure_write_fault_writer)

        snake_add_catch2_test(secure_write_fault_tests
            SOURCES tests/SecureWriteFaultTests.cpp
            LIBS snake_persistence)
        target_compile_definitions(secure_write_fault_tests PRIVATE
            SNAKE_FAULT_LIBRARY="$<TARGET_FILE:snake_fault_inject>"
            SNAKE_FAULT_WRITER="$<TARGET_FILE:secure_write_fault_writer>")
        add_dependencies(secure_write_fault_tests snake_fault_inject secure_write_fault_writer)

        # Settings too, once they save through SecureFileStore.
        if(TARGET snake_settings)
            get_target_property(snake_settings_links snake_settings LINK_LIBRARIES)
            if(snake_settings_links AND "snake_persistence" IN_LIST snake_settings_links)
                foreach(target secure_write_fault_writer secure_write_fault_tests)
                    target_link_libraries(${target} PRIVATE snake_settings)
                    target_compile_definitions(${target} PRIVATE SNAKE_HAVE_SETTINGS)
                endforeach()
            endif()
        endif()
    endif()

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
