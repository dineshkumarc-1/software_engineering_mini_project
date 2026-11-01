// Native Windows TC-SEC-04 tests. The parent process inspects persistence
// after a child writer is abruptly terminated or receives a real API failure.

#include "snakextreme/GameMode.hpp"
#include "snakextreme/LeaderboardRepository.hpp"
#include "snakextreme/SecureFileStore.hpp"
#include "snakextreme/Settings.hpp"
#include "snakextreme/SettingsRepository.hpp"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>

#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

#include <atomic>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <vector>

#ifndef SNAKE_WINDOWS_FAULT_WRITER
#error SNAKE_WINDOWS_FAULT_WRITER must name the child writer executable
#endif
#ifndef SNAKE_WINDOWS_FAULT_DLL
#error SNAKE_WINDOWS_FAULT_DLL must name the test-only injector DLL
#endif

namespace fs = std::filesystem;
using namespace std::chrono_literals;

namespace {

constexpr DWORD InjectedCrashExitCode = 0xE241;

class TempDir {
public:
    TempDir() : path_(makePath()) {
        std::error_code error;
        fs::create_directories(path_, error);
        if (error) {
            throw fs::filesystem_error("Could not create fault-test directory", path_, error);
        }
    }

    ~TempDir() {
        std::error_code ignored;
        fs::remove_all(path_, ignored);
    }

    TempDir(const TempDir&) = delete;
    TempDir& operator=(const TempDir&) = delete;

    const fs::path& path() const noexcept { return path_; }
    fs::path file(const wchar_t* name) const { return path_ / name; }

private:
    static fs::path makePath() {
        static std::atomic<unsigned long> sequence{0};
        const auto id = sequence.fetch_add(1, std::memory_order_relaxed);
        return fs::temp_directory_path() /
            (L"snakextreme-win-fault-" + std::to_wstring(::GetCurrentProcessId()) +
             L"-" + std::to_wstring(::GetTickCount64()) + L"-" + std::to_wstring(id));
    }

    fs::path path_;
};

std::string readBytes(const fs::path& path) {
    std::ifstream input(path, std::ios::binary);
    return {std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
}

void writeBytes(const fs::path& path, const std::string& bytes) {
    std::ofstream output(path, std::ios::binary | std::ios::trunc);
    output.write(bytes.data(), static_cast<std::streamsize>(bytes.size()));
    output.close();
    if (!output) {
        throw std::runtime_error("Could not write the initial fault-test fixture");
    }
}

bool holds(const fs::path& path, const std::string& expected) {
    return readBytes(path) == expected;
}

std::string boardJson(const std::vector<int>& scores) {
    nlohmann::json document{{"version", 1}, {"entries", nlohmann::json::array()}};
    int index = 0;
    for (const int score : scores) {
        document["entries"].push_back({
            {"player_name", "P" + std::to_string(++index)},
            {"score", score},
            {"game_mode", "Classic"},
            {"difficulty", "Medium"},
            {"date_achieved", "2026-10-28"}});
    }
    return document.dump(2);
}

bool endsWith(const std::wstring& value, const std::wstring& suffix) {
    return value.size() >= suffix.size() &&
        value.compare(value.size() - suffix.size(), suffix.size(), suffix) == 0;
}

std::vector<fs::path> temporaries(const fs::path& directory, const wchar_t* destinationName) {
    std::vector<fs::path> found;
    const std::wstring prefix = std::wstring(destinationName) + L".tmp.";
    for (const auto& item : fs::directory_iterator(directory)) {
        const std::wstring name = item.path().filename().wstring();
        if (name.compare(0, prefix.size(), prefix) == 0 && !endsWith(name, L".lock")) {
            found.push_back(item.path());
        }
    }
    return found;
}

std::vector<fs::path> lockSidecars(const fs::path& directory, const wchar_t* destinationName) {
    std::vector<fs::path> found;
    const std::wstring prefix = std::wstring(destinationName) + L".tmp.";
    for (const auto& item : fs::directory_iterator(directory)) {
        const std::wstring name = item.path().filename().wstring();
        if (name.compare(0, prefix.size(), prefix) == 0 && endsWith(name, L".lock")) {
            found.push_back(item.path());
        }
    }
    return found;
}

void ageLeftovers(const fs::path& directory, const wchar_t* destinationName,
    std::chrono::hours age) {
    const auto oldTime = fs::file_time_type::clock::now() - age;
    for (const auto& path : temporaries(directory, destinationName)) {
        fs::last_write_time(path, oldTime);
    }
    for (const auto& path : lockSidecars(directory, destinationName)) {
        fs::last_write_time(path, oldTime);
    }
}

std::wstring quoteArgument(const std::wstring& argument) {
    std::wstring quoted = L"\"";
    std::size_t backslashes = 0;
    for (const wchar_t character : argument) {
        if (character == L'\\') {
            ++backslashes;
        } else if (character == L'\"') {
            quoted.append(backslashes * 2 + 1, L'\\');
            quoted.push_back(character);
            backslashes = 0;
        } else {
            quoted.append(backslashes, L'\\');
            quoted.push_back(character);
            backslashes = 0;
        }
    }
    quoted.append(backslashes * 2, L'\\');
    quoted.push_back(L'\"');
    return quoted;
}

struct ChildResult {
    DWORD exitCode{static_cast<DWORD>(-1)};
};

ChildResult runWriter(const wchar_t* kind, const fs::path& target, const wchar_t* fault) {
    const fs::path writer(SNAKE_WINDOWS_FAULT_WRITER);
    const fs::path injector(SNAKE_WINDOWS_FAULT_DLL);
    const std::wstring writerString = writer.wstring();
    const std::wstring targetString = target.wstring();
    const std::wstring injectorString = injector.wstring();

    std::wstring commandLine = quoteArgument(writerString) + L" " +
        quoteArgument(kind) + L" " + quoteArgument(targetString) + L" " +
        quoteArgument(fault) + L" " + quoteArgument(injectorString);
    std::vector<wchar_t> mutableCommandLine(commandLine.begin(), commandLine.end());
    mutableCommandLine.push_back(L'\0');

    STARTUPINFOW startup{};
    startup.cb = sizeof(startup);
    PROCESS_INFORMATION process{};
    const std::wstring workingDirectory = target.parent_path().wstring();

    const BOOL started = ::CreateProcessW(
        writerString.c_str(), mutableCommandLine.data(), nullptr, nullptr, FALSE,
        CREATE_NO_WINDOW, nullptr, workingDirectory.c_str(), &startup, &process);
    REQUIRE(started != FALSE);

    const DWORD waitResult = ::WaitForSingleObject(process.hProcess, 30000);
    if (waitResult == WAIT_TIMEOUT) {
        (void)::TerminateProcess(process.hProcess, 0xE242);
        (void)::WaitForSingleObject(process.hProcess, 5000);
    }

    ChildResult result;
    const BOOL gotExitCode = ::GetExitCodeProcess(process.hProcess, &result.exitCode);
    ::CloseHandle(process.hThread);
    ::CloseHandle(process.hProcess);

    REQUIRE(waitResult == WAIT_OBJECT_0);
    REQUIRE(gotExitCode != FALSE);
    return result;
}

struct CrashPoint {
    const wchar_t* fault;
    const char* description;
    bool renamed;
};

const CrashPoint CrashPoints[] = {
    {L"crash-after-open", "after temporary file creation", false},
    {L"crash-after-write", "after writing, before flush", false},
    {L"crash-after-flush", "after flush, before replacement", false},
    {L"crash-after-rename", "after replacement", true},
};

const wchar_t* const FailedCalls[] = {
    L"fail-write", L"fail-flush", L"fail-rename"};

} // namespace

TEST_CASE("TC-SEC-04 Windows: abrupt termination leaves the old or new valid leaderboard",
    "[security][TC-SEC-04][fault-injection][windows]") {
    const std::string oldBoard = boardJson({300, 200, 100});

    for (const CrashPoint& point : CrashPoints) {
        CAPTURE(point.description);
        TempDir directory;
        const fs::path board = directory.file(L"leaderboard.json");
        writeBytes(board, oldBoard);

        const ChildResult child = runWriter(L"leaderboard", board, point.fault);
        REQUIRE(child.exitCode == InjectedCrashExitCode);
        REQUIRE(snake::SecureFileStore(board).readJson().status == snake::StoreReadStatus::Ok);

        const auto entries = snake::LeaderboardRepository(board).top10();
        if (point.renamed) {
            REQUIRE(entries.size() == 4);
            REQUIRE(entries[0].playerName == "Crash");
            REQUIRE(entries[0].score == 400);
            REQUIRE(temporaries(directory.path(), L"leaderboard.json").empty());
        } else {
            REQUIRE(holds(board, oldBoard));
            REQUIRE(entries.size() == 3);
            const auto left = temporaries(directory.path(), L"leaderboard.json");
            REQUIRE(left.size() == 1);
            if (std::wstring(point.fault) == L"crash-after-open") {
                REQUIRE(fs::file_size(left.front()) == 0);
            } else {
                REQUIRE(nlohmann::json::parse(readBytes(left.front())).at("entries").size() == 4);
            }
            for (const auto& sidecar : lockSidecars(directory.path(), L"leaderboard.json")) {
                REQUIRE(sidecar == fs::path(left.front().wstring() + L".lock"));
            }
        }
    }
}

TEST_CASE("TC-SEC-04 Windows: a subsequent save succeeds after each abrupt termination",
    "[security][TC-SEC-04][fault-injection][windows]") {
    for (const CrashPoint& point : CrashPoints) {
        CAPTURE(point.description);
        TempDir directory;
        const fs::path board = directory.file(L"leaderboard.json");
        writeBytes(board, boardJson({300, 200, 100}));
        REQUIRE(runWriter(L"leaderboard", board, point.fault).exitCode == InjectedCrashExitCode);

        snake::LeaderboardRepository repository(board);
        REQUIRE(repository.addScore("Next", 500, snake::GameMode::Timed,
            snake::Difficulty::Hard).status == snake::ScoreSubmissionStatus::Submitted);
        const auto entries = repository.top10();
        REQUIRE(entries.size() == (point.renamed ? 5u : 4u));
        REQUIRE(entries[0].playerName == "Next");
        REQUIRE(entries.back().score == 100);
    }
}

TEST_CASE("TC-SEC-04 Windows: failed write, flush and replacement preserve the old board",
    "[security][TC-SEC-04][fault-injection][windows]") {
    const std::string oldBoard = boardJson({300, 200, 100});
    for (const wchar_t* fault : FailedCalls) {
        CAPTURE(std::wstring(fault));
        TempDir directory;
        const fs::path board = directory.file(L"leaderboard.json");
        writeBytes(board, oldBoard);

        const ChildResult child = runWriter(L"leaderboard", board, fault);
        REQUIRE(child.exitCode == 3);
        REQUIRE(holds(board, oldBoard));
        REQUIRE(snake::LeaderboardRepository(board).top10().size() == 3);
        REQUIRE(temporaries(directory.path(), L"leaderboard.json").empty());
        REQUIRE(lockSidecars(directory.path(), L"leaderboard.json").empty());
    }
}

TEST_CASE("TC-SEC-04 Windows: failed first save creates no destination or leftovers",
    "[security][TC-SEC-04][fault-injection][windows]") {
    for (const wchar_t* fault : FailedCalls) {
        CAPTURE(std::wstring(fault));
        TempDir directory;
        const fs::path board = directory.file(L"leaderboard.json");
        const ChildResult child = runWriter(L"leaderboard", board, fault);
        REQUIRE(child.exitCode == 3);
        REQUIRE_FALSE(fs::exists(board));
        REQUIRE(temporaries(directory.path(), L"leaderboard.json").empty());
        REQUIRE(lockSidecars(directory.path(), L"leaderboard.json").empty());
    }
}

TEST_CASE("TC-SEC-04 Windows: normal control save works without an injected fault",
    "[security][TC-SEC-04][fault-injection][windows]") {
    TempDir directory;
    const fs::path board = directory.file(L"leaderboard.json");
    writeBytes(board, boardJson({300}));
    const ChildResult child = runWriter(L"leaderboard", board, L"none");
    REQUIRE(child.exitCode == 0);
    REQUIRE(snake::LeaderboardRepository(board).top10().size() == 2);
    REQUIRE(temporaries(directory.path(), L"leaderboard.json").empty());
    REQUIRE(lockSidecars(directory.path(), L"leaderboard.json").empty());
}

TEST_CASE("TC-SEC-04 Windows: crash leftovers are removed after the agreed 24-hour age",
    "[security][TC-SEC-04][fault-injection][windows]") {
    for (const wchar_t* fault : {L"crash-after-flush", L"crash-after-rename"}) {
        CAPTURE(std::wstring(fault));
        TempDir directory;
        const fs::path board = directory.file(L"leaderboard.json");
        writeBytes(board, boardJson({300, 200, 100}));
        REQUIRE(runWriter(L"leaderboard", board, fault).exitCode == InjectedCrashExitCode);
        ageLeftovers(directory.path(), L"leaderboard.json", 48h);

        snake::LeaderboardRepository repository(board);
        (void)repository.top10();
        REQUIRE(repository.addScore("Next", 500, snake::GameMode::Timed,
            snake::Difficulty::Hard).status == snake::ScoreSubmissionStatus::Submitted);
        REQUIRE(temporaries(directory.path(), L"leaderboard.json").empty());
        REQUIRE(lockSidecars(directory.path(), L"leaderboard.json").empty());
    }
}

TEST_CASE("TC-SEC-04 Windows: settings retain old or new valid data across injected crashes",
    "[security][TC-SEC-04][fault-injection][windows][settings]") {
    snake::Settings oldSettings;
    oldSettings.theme = snake::Theme::Retro;
    oldSettings.musicVolume = 80;

    for (const CrashPoint& point : CrashPoints) {
        CAPTURE(point.description);
        TempDir directory;
        const fs::path config = directory.file(L"config.json");
        std::string error;
        REQUIRE(snake::SettingsRepository(config).save(oldSettings, error));
        const std::string oldBytes = readBytes(config);

        REQUIRE(runWriter(L"settings", config, point.fault).exitCode == InjectedCrashExitCode);
        const auto loaded = snake::SettingsRepository(config).load();
        REQUIRE_FALSE(loaded.usedDefaults);
        if (point.renamed) {
            REQUIRE(loaded.settings.theme == snake::Theme::Neon);
            REQUIRE(loaded.settings.musicVolume == 15);
        } else {
            REQUIRE(holds(config, oldBytes));
            REQUIRE(loaded.settings == oldSettings);
        }
    }

    for (const wchar_t* fault : FailedCalls) {
        CAPTURE(std::wstring(fault));
        TempDir directory;
        const fs::path config = directory.file(L"config.json");
        std::string error;
        REQUIRE(snake::SettingsRepository(config).save(oldSettings, error));
        const std::string oldBytes = readBytes(config);

        const ChildResult child = runWriter(L"settings", config, fault);
        REQUIRE(child.exitCode == 3);
        REQUIRE(holds(config, oldBytes));
        REQUIRE(snake::SettingsRepository(config).load().settings == oldSettings);
        REQUIRE(temporaries(directory.path(), L"config.json").empty());
        REQUIRE(lockSidecars(directory.path(), L"config.json").empty());
    }
}
