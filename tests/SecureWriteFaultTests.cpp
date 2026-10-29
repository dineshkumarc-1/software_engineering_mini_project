// Issue #18, TC-SEC-04 with real fault injection (Linux only).
//
// tests/fault/FaultWriter.cpp performs one save in a child process with the
// LD_PRELOAD library tests/fault/FaultInjection.cpp, which either aborts the
// process at one of the four STP points of write-temp-then-rename or makes one
// system call of the save fail. The "restart" is this test process reading
// the files afterwards with a fresh repository.
//
// Unlike the "simulated leftovers" test in SecurityCorpusTests.cpp, which only
// writes the files a crash would leave, these tests crash or fail the actual
// SecureFileStore::writeJson code.

#include "snakextreme/GameMode.hpp"
#include "snakextreme/LeaderboardRepository.hpp"
#include "snakextreme/SecureFileStore.hpp"

#if defined(SNAKE_HAVE_SETTINGS)
#include "snakextreme/Settings.hpp"
#include "snakextreme/SettingsRepository.hpp"
#endif

#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <algorithm>
#include <chrono>
#include <csignal>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <random>
#include <string>
#include <vector>

extern char** environ;

namespace fs = std::filesystem;
using namespace std::chrono_literals;

using snake::Difficulty;
using snake::GameMode;
using snake::LeaderboardRepository;
using snake::ScoreSubmissionStatus;
using snake::SecureFileStore;
using snake::StoreReadStatus;

namespace {

class TempDir {
public:
    TempDir() : path_(uniquePath()) {
        fs::create_directories(path_);
    }
    ~TempDir() {
        std::error_code ignored;
        fs::remove_all(path_, ignored);
    }
    TempDir(const TempDir&) = delete;
    TempDir& operator=(const TempDir&) = delete;

    const fs::path& path() const noexcept {
        return path_;
    }
    fs::path file(const char* name) const {
        return path_ / name;
    }

private:
    static fs::path uniquePath() {
        std::random_device random;
        return fs::temp_directory_path() /
               ("snakextreme-fault-" + std::to_string(random()) + "-" + std::to_string(random()));
    }

    fs::path path_;
};

std::string readBytes(const fs::path& path) {
    std::ifstream input(path, std::ios::binary);
    return {std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
}

bool holds(const fs::path& path, const std::string& expected) {
    return readBytes(path) == expected;
}

void writeBytes(const fs::path& path, const std::string& bytes) {
    std::ofstream output(path, std::ios::binary | std::ios::trunc);
    output << bytes;
}

std::string boardJson(const std::vector<int>& scores) {
    nlohmann::json document{{"version", 1}, {"entries", nlohmann::json::array()}};
    int index = 0;
    for (const int score : scores) {
        document["entries"].push_back({{"player_name", "P" + std::to_string(++index)},
                                       {"score", score},
                                       {"game_mode", "Classic"},
                                       {"difficulty", "Medium"},
                                       {"date_achieved", "2026-10-28"}});
    }
    return document.dump(2);
}

bool endsWithLock(const std::string& name) {
    const std::string suffix = ".lock";
    return name.size() > suffix.size() &&
           name.compare(name.size() - suffix.size(), suffix.size(), suffix) == 0;
}

// Temporary files "<file>.tmp.<nonce>" SecureFileStore left in `dir`.
std::vector<fs::path> temporaries(const fs::path& dir) {
    std::vector<fs::path> found;
    for (const auto& item : fs::directory_iterator(dir)) {
        const std::string name = item.path().filename().string();
        if (name.find(".tmp.") != std::string::npos && !endsWithLock(name)) {
            found.push_back(item.path());
        }
    }
    return found;
}

// Lock sidecars "<file>.tmp.<nonce>.lock", if the store uses them.
std::vector<fs::path> lockSidecars(const fs::path& dir) {
    std::vector<fs::path> found;
    for (const auto& item : fs::directory_iterator(dir)) {
        const std::string name = item.path().filename().string();
        if (name.find(".tmp.") != std::string::npos && endsWithLock(name)) {
            found.push_back(item.path());
        }
    }
    return found;
}

// Makes everything a crash left in `dir` look `age` old.
void ageLeftovers(const fs::path& dir, std::chrono::hours age) {
    for (const fs::path& path : temporaries(dir)) {
        fs::last_write_time(path, fs::file_time_type::clock::now() - age);
    }
    for (const fs::path& path : lockSidecars(dir)) {
        fs::last_write_time(path, fs::file_time_type::clock::now() - age);
    }
}

struct ChildResult {
    bool aborted{false}; // ended by SIGABRT (an injected crash)
    int exitCode{-1};    // exit code when it exited normally
};

// Runs the writer once with the fault injector and `fault`.
ChildResult runWriter(const std::string& kind, const fs::path& target, const std::string& fault) {
    std::vector<std::string> environment;
    for (char** entry = environ; *entry != nullptr; ++entry) {
        const std::string value(*entry);
        if (value.rfind("LD_PRELOAD=", 0) != 0 && value.rfind("SNAKE_FAULT=", 0) != 0 &&
            value.rfind("ASAN_OPTIONS=", 0) != 0) {
            environment.push_back(value);
        }
    }
    // A preloaded library comes before a dynamic ASan runtime (GCC builds).
    const char* asan = std::getenv("ASAN_OPTIONS");
    environment.push_back(std::string("ASAN_OPTIONS=") +
                          (asan != nullptr && *asan != '\0' ? std::string(asan) + ":" : "") +
                          "verify_asan_link_order=0");
    environment.push_back(std::string("LD_PRELOAD=") + SNAKE_FAULT_LIBRARY);
    environment.push_back("SNAKE_FAULT=" + fault);

    // execve() takes char* for historical reasons; it does not modify them.
    std::vector<char*> envp;
    std::transform(environment.begin(), environment.end(), std::back_inserter(envp),
                   [](const std::string& value) { return const_cast<char*>(value.c_str()); });
    envp.push_back(nullptr);

    const pid_t pid = ::fork();
    REQUIRE(pid >= 0);
    if (pid == 0) {
        const std::string writer = SNAKE_FAULT_WRITER;
        const std::string targetArgument = target.string();
        char* const argv[] = {const_cast<char*>(writer.c_str()), const_cast<char*>(kind.c_str()),
                              const_cast<char*>(targetArgument.c_str()), nullptr};
        const rlimit noCoreFile{0, 0};
        (void)::setrlimit(RLIMIT_CORE, &noCoreFile);
        ::execve(writer.c_str(), argv, envp.data());
        ::_exit(127);
    }
    int status = 0;
    REQUIRE(::waitpid(pid, &status, 0) == pid);

    ChildResult result;
    result.aborted = WIFSIGNALED(status) && WTERMSIG(status) == SIGABRT;
    result.exitCode = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
    REQUIRE(result.exitCode != 127); // the writer could be started
    return result;
}

struct CrashPoint {
    const char* fault;
    const char* stp;
    bool renamed;  // the new board is in place
};

const CrashPoint CrashPoints[] = {
    {"crash-after-open", "(a) after the temporary file was opened", false},
    {"crash-after-write", "(b) after it was written, before the flush", false},
    {"crash-after-flush", "(c) after the flush, before the rename", false},
    {"crash-after-rename", "(d) after the rename", true},
};

const char* const FailedCalls[] = {"fail-write", "fail-flush", "fail-rename"};

} // namespace

// ---------------------------------------------------------------------------
// Leaderboard (C11 over C13)
// ---------------------------------------------------------------------------

TEST_CASE("TC-SEC-04 injected crash: a save aborted at each step leaves the old or the new board",
          "[security][TC-SEC-04][fault-injection]") {
    const std::string oldBoard = boardJson({300, 200, 100});

    for (const CrashPoint& point : CrashPoints) {
        CAPTURE(point.stp);
        TempDir dir;
        const fs::path board = dir.file("leaderboard.json");
        writeBytes(board, oldBoard);

        const ChildResult child = runWriter("leaderboard", board, point.fault);
        REQUIRE(child.aborted);

        // Restart: never partial or empty, always valid.
        REQUIRE(SecureFileStore(board).readJson().status == StoreReadStatus::Ok);
        const auto entries = LeaderboardRepository(board).top10();
        if (point.renamed) {
            REQUIRE(entries.size() == 4);
            REQUIRE(entries[0].playerName == "Crash");
            REQUIRE(entries[0].score == 400);
            REQUIRE(temporaries(dir.path()).empty());
        } else {
            REQUIRE(holds(board, oldBoard)); // byte for byte
            REQUIRE(entries.size() == 3);
            REQUIRE(entries[0].score == 300);
            // The crash really happened inside the save: its temporary file
            // is still there (empty right after opening it).
            const auto left = temporaries(dir.path());
            REQUIRE(left.size() == 1);
            if (std::string(point.fault) == "crash-after-open") {
                REQUIRE(fs::file_size(left.front()) == 0);
            } else {
                REQUIRE(nlohmann::json::parse(readBytes(left.front())).at("entries").size() == 4);
            }
            // A lock sidecar, if the store uses one, belongs to that file.
            for (const fs::path& sidecar : lockSidecars(dir.path())) {
                CAPTURE(sidecar.filename().string());
                REQUIRE(sidecar.string() == left.front().string() + ".lock");
            }
        }
    }
}

TEST_CASE("TC-SEC-04 injected crash: the next save after a crash works and keeps every score",
          "[security][TC-SEC-04][fault-injection]") {
    for (const CrashPoint& point : CrashPoints) {
        CAPTURE(point.stp);
        TempDir dir;
        const fs::path board = dir.file("leaderboard.json");
        writeBytes(board, boardJson({300, 200, 100}));
        REQUIRE(runWriter("leaderboard", board, point.fault).aborted);

        LeaderboardRepository repository(board);
        REQUIRE(repository.addScore("Next", 500, GameMode::Timed, Difficulty::Hard).status ==
                ScoreSubmissionStatus::Submitted);
        const auto entries = repository.top10();
        REQUIRE(entries.size() == (point.renamed ? 5u : 4u));
        REQUIRE(entries[0].playerName == "Next");
        REQUIRE(entries.back().score == 100);
    }
}

TEST_CASE("TC-SEC-04 injected failure: a failed write, flush or rename keeps the old board",
          "[security][TC-SEC-04][fault-injection]") {
    const std::string oldBoard = boardJson({300, 200, 100});

    for (const char* fault : FailedCalls) {
        CAPTURE(fault);
        TempDir dir;
        const fs::path board = dir.file("leaderboard.json");
        writeBytes(board, oldBoard);

        const ChildResult child = runWriter("leaderboard", board, fault);
        REQUIRE_FALSE(child.aborted);
        REQUIRE(child.exitCode == 3); // the save reports an error
        REQUIRE(holds(board, oldBoard));
        REQUIRE(LeaderboardRepository(board).top10().size() == 3);
        REQUIRE(temporaries(dir.path()).empty()); // the failed save cleans up
        REQUIRE(lockSidecars(dir.path()).empty());
    }
}

TEST_CASE("TC-SEC-04 injected failure: a failed first save creates no file",
          "[security][TC-SEC-04][fault-injection]") {
    for (const char* fault : FailedCalls) {
        CAPTURE(fault);
        TempDir dir;
        const fs::path board = dir.file("leaderboard.json");

        const ChildResult child = runWriter("leaderboard", board, fault);
        REQUIRE(child.exitCode == 3);
        REQUIRE_FALSE(fs::exists(board));
        REQUIRE(temporaries(dir.path()).empty());
        REQUIRE(lockSidecars(dir.path()).empty());
    }
}

TEST_CASE("TC-SEC-04 without a fault the writer saves normally (control)",
          "[security][TC-SEC-04][fault-injection]") {
    TempDir dir;
    const fs::path board = dir.file("leaderboard.json");
    writeBytes(board, boardJson({300}));
    const ChildResult child = runWriter("leaderboard", board, "none");
    REQUIRE_FALSE(child.aborted);
    REQUIRE(child.exitCode == 0);
    REQUIRE(LeaderboardRepository(board).top10().size() == 2);
    REQUIRE(temporaries(dir.path()).empty());
    REQUIRE(lockSidecars(dir.path()).empty());
}

TEST_CASE("TC-SEC-04 injected crash: a fresh crash leftover is kept, so cleanup never races a live save",
          "[security][TC-SEC-04][fault-injection]") {
    // Agreed cleanup policy (#48): only leftovers older than 24 hours are
    // removed. A crash moments ago is indistinguishable from a save still in
    // progress, so the next load and save keep its files. This is why a
    // crash leftover is not removed on the very next start.
    TempDir dir;
    const fs::path board = dir.file("leaderboard.json");
    writeBytes(board, boardJson({300, 200, 100}));
    REQUIRE(runWriter("leaderboard", board, "crash-after-flush").aborted);
    const auto left = temporaries(dir.path());
    REQUIRE(left.size() == 1);
    const std::string leftBytes = readBytes(left.front());

    LeaderboardRepository repository(board);
    REQUIRE(repository.top10().size() == 3);
    REQUIRE(repository.addScore("Next", 500, GameMode::Timed, Difficulty::Hard).status ==
            ScoreSubmissionStatus::Submitted);
    REQUIRE(repository.top10().size() == 4);
    REQUIRE(fs::exists(left.front()));
    REQUIRE(holds(left.front(), leftBytes));
}

TEST_CASE("Known defect D5 (injected crash): a crashed save's temporary file is never removed",
          "[security][TC-SEC-04][fault-injection][known-defect][!shouldfail]") {
    // STP TC-SEC-04 "stale .tmp removed" (SAD 4.3.4 removeStaleTemp). The
    // agreed policy (#48) removes leftovers older than 24 hours, so what the
    // crash left is made 48 hours old first. Both crash points that leave
    // something behind are covered: before the rename (a temporary file and
    // its lock sidecar) and after it (only a lock sidecar, if the store uses
    // one).
    for (const char* fault : {"crash-after-flush", "crash-after-rename"}) {
        CAPTURE(fault);
        TempDir dir;
        const fs::path board = dir.file("leaderboard.json");
        writeBytes(board, boardJson({300, 200, 100}));
        REQUIRE(runWriter("leaderboard", board, fault).aborted);
        if (std::string(fault) == "crash-after-flush") {
            REQUIRE(temporaries(dir.path()).size() == 1);
        }
        ageLeftovers(dir.path(), 48h);

        LeaderboardRepository repository(board);
        (void)repository.top10();
        REQUIRE(repository.addScore("Next", 500, GameMode::Timed, Difficulty::Hard).status ==
                ScoreSubmissionStatus::Submitted);
        REQUIRE(temporaries(dir.path()).empty());
        REQUIRE(lockSidecars(dir.path()).empty());
    }
}

// ---------------------------------------------------------------------------
// Settings (C12 over C13), when settings save through SecureFileStore
// ---------------------------------------------------------------------------

#if defined(SNAKE_HAVE_SETTINGS)

TEST_CASE("TC-SEC-04 injected crash and failure: config.json keeps old or new settings",
          "[security][TC-SEC-04][fault-injection][settings]") {
    snake::Settings old;
    old.theme = snake::Theme::Retro;
    old.musicVolume = 80;

    for (const CrashPoint& point : CrashPoints) {
        CAPTURE(point.stp);
        TempDir dir;
        const fs::path config = dir.file("config.json");
        std::string error;
        REQUIRE(snake::SettingsRepository(config).save(old, error));
        const std::string oldBytes = readBytes(config);

        REQUIRE(runWriter("settings", config, point.fault).aborted);
        const auto loaded = snake::SettingsRepository(config).load();
        REQUIRE_FALSE(loaded.usedDefaults);
        if (point.renamed) {
            REQUIRE(loaded.settings.theme == snake::Theme::Neon);
            REQUIRE(loaded.settings.musicVolume == 15);
        } else {
            REQUIRE(holds(config, oldBytes));
            REQUIRE(loaded.settings == old);
        }
    }

    for (const char* fault : FailedCalls) {
        CAPTURE(fault);
        TempDir dir;
        const fs::path config = dir.file("config.json");
        std::string error;
        REQUIRE(snake::SettingsRepository(config).save(old, error));
        const std::string oldBytes = readBytes(config);

        const ChildResult child = runWriter("settings", config, fault);
        REQUIRE(child.exitCode == 3);
        REQUIRE(holds(config, oldBytes));
        REQUIRE(snake::SettingsRepository(config).load().settings == old);
        REQUIRE(temporaries(dir.path()).empty());
        REQUIRE(lockSidecars(dir.path()).empty());
    }
}

#endif
