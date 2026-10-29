// Issue #18: execution of the leaderboard and security test cases against
// the C11/C13 persistence code (SecureFileStore, LeaderboardRepository) and
// the C10 name validator.
//
//  - TC-SEC-03 / acceptance 1: the 25 valid + 25 malformed leaderboard.json
//    fixtures in tests/fixtures/json-corpus/leaderboard (each load <= 1 s).
//  - SEC-007 / acceptance 2: exactly 64 KiB loads, 64 KiB + 1 byte is
//    refused, and a huge file is refused without being read.
//  - TC-LB-01, TC-LB-03, TC-LB-04, TC-SEC-01, TC-SEC-04, TC-SEC-05 and
//    TC-SEC-06 at the repository level.
//
// Behaviour that the SRS/SAD/STP require but the code does not do yet is
// kept as "[!shouldfail]" tests named "Known defect ...": they pass while
// the defect exists and start failing once it is fixed, as a reminder to
// drop the tag. Tests marked (D1)...(D5) started that way and became
// ordinary regression tests with the persistence fix in #48.
// See docs/qa/TC-PWR-LB-SEC-EXECUTION.md.

#include "snakextreme/GameMode.hpp"
#include "snakextreme/InputValidator.hpp"
#include "snakextreme/LeaderboardRepository.hpp"
#include "snakextreme/SecureFileStore.hpp"

#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <random>
#include <set>
#include <string>
#include <string_view>
#include <vector>

#if defined(_WIN32)
#include <io.h>
#include <share.h>
#include <fcntl.h>
#include <sys/stat.h>
#else
#include <fcntl.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

namespace fs = std::filesystem;
using namespace std::chrono_literals;

using snake::Difficulty;
using snake::GameMode;
using snake::LeaderboardEntry;
using snake::LeaderboardRepository;
using snake::ScoreSubmissionStatus;
using snake::SecureFileStore;
using snake::StoreReadStatus;

namespace {

const fs::path CorpusDir{SNAKE_JSON_CORPUS_DIR};

// A fresh directory for one test, removed afterwards.
class TempDir {
public:
    TempDir() : path_(uniquePath()) {
        fs::create_directories(path_);
    }
    ~TempDir() {
        std::error_code ignored;
        fs::permissions(path_, fs::perms::owner_all, fs::perm_options::add, ignored);
        fs::remove_all(path_, ignored);
    }
    TempDir(const TempDir&) = delete;
    TempDir& operator=(const TempDir&) = delete;

    const fs::path& path() const noexcept {
        return path_;
    }
    fs::path board() const {
        return path_ / "leaderboard.json";
    }

private:
    static fs::path uniquePath() {
        std::random_device random;
        return fs::temp_directory_path() /
               ("snakextreme-corpus-" + std::to_string(random()) + "-" + std::to_string(random()));
    }

    fs::path path_;
};

std::vector<fs::path> fixtures(const std::string& set) {
    std::vector<fs::path> files;
    for (const auto& item : fs::directory_iterator(CorpusDir / "leaderboard" / set)) {
        if (item.path().extension() == ".json") {
            files.push_back(item.path());
        }
    }
    std::sort(files.begin(), files.end());
    return files;
}

std::string readBytes(const fs::path& path) {
    std::ifstream input(path, std::ios::binary);
    return {std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
}

// True when the file still holds exactly `expected`.
bool holds(const fs::path& path, const std::string& expected) {
    return readBytes(path) == expected;
}

void writeBytes(const fs::path& path, const std::string& bytes) {
    std::ofstream output(path, std::ios::binary | std::ios::trunc);
    output << bytes;
}

void install(const fs::path& fixture, const fs::path& target) {
    fs::copy_file(fixture, target, fs::copy_options::overwrite_existing);
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

struct Timed {
    std::vector<LeaderboardEntry> entries;
    std::chrono::milliseconds took{0};
};

Timed timedTop10(const fs::path& board) {
    const auto start = std::chrono::steady_clock::now();
    Timed result;
    result.entries = LeaderboardRepository(board).top10();
    result.took = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start);
    return result;
}

// Makes `path` look `age` old (its last write time).
void age(const fs::path& path, std::chrono::hours hours) {
    fs::last_write_time(path, fs::file_time_type::clock::now() - hours);
}

#if !defined(_WIN32)
// Ages a symbolic link itself, not the file it points to.
bool ageLink(const fs::path& link, std::chrono::hours hours) {
    const auto when = std::chrono::system_clock::now() - hours;
    const auto seconds =
        std::chrono::duration_cast<std::chrono::seconds>(when.time_since_epoch()).count();
    const timespec times[2] = {{static_cast<time_t>(seconds), 0},
                               {static_cast<time_t>(seconds), 0}};
    return ::utimensat(AT_FDCWD, link.c_str(), times, AT_SYMLINK_NOFOLLOW) == 0;
}
#endif

// Holds `path` the way a live save holds its lock sidecar: an exclusive
// flock on POSIX, an open handle that shares nothing on Windows.
class HeldLock {
public:
    explicit HeldLock(const fs::path& path) {
#if defined(_WIN32)
        if (_wsopen_s(&descriptor_, path.c_str(), _O_RDWR | _O_BINARY, _SH_DENYRW,
                      _S_IREAD | _S_IWRITE) != 0) {
            descriptor_ = -1;
        }
#else
        descriptor_ = ::open(path.c_str(), O_RDWR);
        if (descriptor_ >= 0 && ::flock(descriptor_, LOCK_EX | LOCK_NB) != 0) {
            ::close(descriptor_);
            descriptor_ = -1;
        }
#endif
    }
    ~HeldLock() {
        if (descriptor_ >= 0) {
#if defined(_WIN32)
            _close(descriptor_);
#else
            ::flock(descriptor_, LOCK_UN);
            ::close(descriptor_);
#endif
        }
    }
    HeldLock(const HeldLock&) = delete;
    HeldLock& operator=(const HeldLock&) = delete;

    bool held() const noexcept {
        return descriptor_ >= 0;
    }

private:
    int descriptor_{-1};
};

bool isIsoDate(std::string_view text) {
    if (text.size() != 10 || text[4] != '-' || text[7] != '-') {
        return false;
    }
    for (std::size_t i = 0; i < text.size(); ++i) {
        if (i != 4 && i != 7 && (text[i] < '0' || text[i] > '9')) {
            return false;
        }
    }
    return true;
}

} // namespace

// ---------------------------------------------------------------------------
// TC-SEC-03, acceptance 1: 25 valid + 25 malformed leaderboard.json files
// ---------------------------------------------------------------------------

TEST_CASE("Corpus: all 25 valid leaderboard files load exactly as written",
          "[corpus][TC-SEC-03][TC-LB-01]") {
    const auto files = fixtures("valid");
    REQUIRE(files.size() == 25);

    for (const fs::path& fixture : files) {
        CAPTURE(fixture.filename().string());
        TempDir dir;
        install(fixture, dir.board());

        const auto read = SecureFileStore(dir.board()).readJson();
        REQUIRE(read.status == StoreReadStatus::Ok);

        // The expected board, read independently from the fixture.
        const auto expected = nlohmann::json::parse(readBytes(fixture)).at("entries");
        const Timed loaded = timedTop10(dir.board());
        REQUIRE(loaded.took < 1s);
        REQUIRE(loaded.entries.size() == expected.size());
        for (std::size_t i = 0; i < loaded.entries.size(); ++i) {
            CAPTURE(i);
            const auto& entry = loaded.entries[i];
            REQUIRE(entry.playerName == expected[i].at("player_name").get<std::string>());
            REQUIRE(entry.score == expected[i].at("score").get<int>());
            REQUIRE(snake::toString(entry.gameMode) ==
                    expected[i].at("game_mode").get<std::string>());
            REQUIRE(snake::toString(entry.difficulty) ==
                    expected[i].at("difficulty").get<std::string>());
            REQUIRE(entry.dateAchieved == expected[i].at("date_achieved").get<std::string>());
        }

        // The board keeps working: a new best score goes in at rank 1.
        LeaderboardRepository repository(dir.board());
        const auto added = repository.addScore("Checker", 2147483647, GameMode::Timed,
                                               Difficulty::Hard);
        if (expected.size() == 1 && expected[0].at("score").get<int>() == 2147483647) {
            REQUIRE(added.status == ScoreSubmissionStatus::Submitted);
            REQUIRE(added.rank == std::size_t{2}); // ties rank after the existing entry
        } else {
            REQUIRE(added.status == ScoreSubmissionStatus::Submitted);
            REQUIRE(added.rank == std::size_t{1});
        }
        REQUIRE(SecureFileStore(dir.board()).readJson().status == StoreReadStatus::Ok);
        REQUIRE(repository.top10().size() == std::min<std::size_t>(expected.size() + 1, 10));
    }
}

TEST_CASE("Corpus: the 25 malformed leaderboard files fall back to an empty board",
          "[corpus][TC-SEC-03][TC-LB-04]") {
    const auto files = fixtures("malformed");
    REQUIRE(files.size() == 25);

    std::size_t checked = 0;
    for (const fs::path& fixture : files) {
        const std::string name = fixture.filename().string();
        CAPTURE(name);
        TempDir dir;
        install(fixture, dir.board());
        const std::string before = readBytes(dir.board());

        // No crash or hang; each load within 1 s.
        const Timed loaded = timedTop10(dir.board());
        REQUIRE(loaded.took < 1s);
        // Loading never changes the file.
        REQUIRE(holds(dir.board(), before));

        REQUIRE(loaded.entries.empty());
        ++checked;
    }
    REQUIRE(checked == 25);
}

TEST_CASE("Corpus: each malformed leaderboard file is rejected for the right reason",
          "[corpus][TC-SEC-03]") {
    struct Expectation {
        const char* file;
        StoreReadStatus status; // what SecureFileStore reports
    };
    // Ok means the JSON itself is fine and the leaderboard schema rejects it.
    const Expectation expectations[] = {
        {"01-empty-file.json", StoreReadStatus::EmptyFile},
        {"02-whitespace-only.json", StoreReadStatus::EmptyFile},
        {"03-truncated.json", StoreReadStatus::InvalidJson},
        {"04-binary.json", StoreReadStatus::InvalidJson},
        {"05-not-json.json", StoreReadStatus::InvalidJson},
        {"06-root-array.json", StoreReadStatus::Ok},
        {"07-root-string.json", StoreReadStatus::Ok},
        {"08-missing-version.json", StoreReadStatus::Ok},
        {"09-wrong-version.json", StoreReadStatus::Ok},
        {"10-version-as-string.json", StoreReadStatus::Ok},
        {"11-missing-entries.json", StoreReadStatus::Ok},
        {"12-entries-is-object.json", StoreReadStatus::Ok},
        {"13-entry-missing-score.json", StoreReadStatus::Ok},
        {"14-score-as-string.json", StoreReadStatus::Ok},
        {"15-negative-score.json", StoreReadStatus::Ok},
        {"16-fractional-score.json", StoreReadStatus::Ok},
        {"17-score-above-int-max.json", StoreReadStatus::Ok},
        {"18-unknown-game-mode.json", StoreReadStatus::Ok},
        {"19-lowercase-difficulty.json", StoreReadStatus::Ok},
        {"20-name-sixteen-chars.json", StoreReadStatus::Ok},
        {"21-name-with-delimiters.json", StoreReadStatus::Ok},
        {"22-empty-name.json", StoreReadStatus::Ok},
        {"23-bad-date-format.json", StoreReadStatus::Ok},
        {"24-eleven-entries.json", StoreReadStatus::Ok},
        {"25-deep-nesting-10000.json", StoreReadStatus::UnsafeJson},
    };
    for (const Expectation& expectation : expectations) {
        CAPTURE(expectation.file);
        const fs::path fixture = CorpusDir / "leaderboard" / "malformed" / expectation.file;
        REQUIRE(fs::exists(fixture));
        REQUIRE(SecureFileStore(fixture).readJson().status == expectation.status);
    }
}

TEST_CASE("TC-SEC-03: a date_achieved that is not a valid YYYY-MM-DD date is rejected (D1)",
          "[corpus][TC-SEC-03]") {
    // SAD 4.3.5: "date_achieved": valid YYYY-MM-DD; any violation -> defaults.
    TempDir dir;
    install(CorpusDir / "leaderboard" / "malformed" / "23-bad-date-format.json", dir.board());
    REQUIRE(LeaderboardRepository(dir.board()).top10().empty());
}

TEST_CASE("TC-LB-04: a board with more than 10 entries is rejected (D2)",
          "[corpus][TC-SEC-03][TC-LB-04]") {
    // SAD 4.3.5: at most 10 entries; STP TC-LB-04 lists "11 entries" as a
    // corrupt fixture that must show an empty leaderboard.
    TempDir dir;
    install(CorpusDir / "leaderboard" / "malformed" / "24-eleven-entries.json", dir.board());
    REQUIRE(LeaderboardRepository(dir.board()).top10().empty());
}

// ---------------------------------------------------------------------------
// SEC-007, acceptance 2: 64 KiB boundary
// ---------------------------------------------------------------------------

TEST_CASE("Boundary: exactly 64 KiB loads, 64 KiB + 1 byte is refused",
          "[corpus][boundary][SEC-007][TC-SEC-03]") {
    REQUIRE(SecureFileStore::MaxFileSizeBytes == 65536);
    const std::string valid = boardJson({300, 200, 100});

    TempDir dir;
    writeBytes(dir.board(), valid + std::string(65536 - valid.size(), ' '));
    REQUIRE(fs::file_size(dir.board()) == 65536);
    REQUIRE(SecureFileStore(dir.board()).readJson().status == StoreReadStatus::Ok);
    REQUIRE(LeaderboardRepository(dir.board()).top10().size() == 3);

    writeBytes(dir.board(), valid + std::string(65537 - valid.size(), ' '));
    REQUIRE(fs::file_size(dir.board()) == 65537);
    const auto refused = SecureFileStore(dir.board()).readJson();
    REQUIRE(refused.status == StoreReadStatus::TooLarge);
    const std::string before = readBytes(dir.board());
    const Timed loaded = timedTop10(dir.board());
    REQUIRE(loaded.entries.empty()); // defaults
    REQUIRE(loaded.took < 1s);
    REQUIRE(holds(dir.board(), before));
}

TEST_CASE("Boundary: a huge file is refused without reading all of it",
          "[corpus][boundary][SEC-007][TC-SEC-03]") {
    TempDir dir;
    writeBytes(dir.board(), boardJson({100}));
    // 256 MiB (sparse where the file system allows): reading it all would
    // take far longer than the limit below.
    fs::resize_file(dir.board(), std::uintmax_t{256} * 1024 * 1024);

    const auto start = std::chrono::steady_clock::now();
    const auto result = SecureFileStore(dir.board()).readJson();
    const auto took = std::chrono::steady_clock::now() - start;
    REQUIRE(result.status == StoreReadStatus::TooLarge);
    REQUIRE(took < 1s);
    REQUIRE(LeaderboardRepository(dir.board()).top10().empty());
}

// ---------------------------------------------------------------------------
// TC-LB-01, TC-LB-03, TC-LB-04
// ---------------------------------------------------------------------------

TEST_CASE("TC-LB-01: five games persist sorted, with every field, across a restart",
          "[leaderboard][TC-LB-01]") {
    TempDir dir;
    {
        LeaderboardRepository repository(dir.board());
        REQUIRE(repository.addScore("Ayushi", 240, GameMode::Classic, Difficulty::Hard).status ==
                ScoreSubmissionStatus::Submitted);
        REQUIRE(repository.addScore("Bharath", 90, GameMode::Timed, Difficulty::Easy).status ==
                ScoreSubmissionStatus::Submitted);
        REQUIRE(repository.addScore("Dinesh", 450, GameMode::Obstacle, Difficulty::Medium)
                    .status == ScoreSubmissionStatus::Submitted);
        REQUIRE(repository.addScore("Tejas", 120, GameMode::Timed, Difficulty::Hard).status ==
                ScoreSubmissionStatus::Submitted);
        REQUIRE(repository.addScore("Guest_1", 15, GameMode::Obstacle, Difficulty::Easy).status ==
                ScoreSubmissionStatus::Submitted);
    }

    // The file: valid JSON, version 1, sorted by score, all five fields.
    const auto document = nlohmann::json::parse(readBytes(dir.board()));
    REQUIRE(document.at("version") == 1);
    const auto& entries = document.at("entries");
    REQUIRE(entries.size() == 5);
    const std::set<std::string> fields{"player_name", "score", "game_mode", "difficulty",
                                       "date_achieved"};
    int previous = 2147483647;
    for (const auto& entry : entries) {
        std::set<std::string> keys;
        for (const auto& item : entry.items()) {
            keys.insert(item.key());
        }
        REQUIRE(keys == fields);
        REQUIRE(entry.at("score").get<int>() <= previous);
        previous = entry.at("score").get<int>();
    }

    // "Restart": a new repository on the same file sees the same board.
    const auto board = LeaderboardRepository(dir.board()).top10();
    REQUIRE(board.size() == 5);
    REQUIRE(board[0].playerName == "Dinesh");
    REQUIRE(board[0].gameMode == GameMode::Obstacle);
    REQUIRE(board[0].difficulty == Difficulty::Medium);
    REQUIRE(board[1].playerName == "Ayushi");
    REQUIRE(board[2].playerName == "Tejas");
    REQUIRE(board[3].playerName == "Bharath");
    REQUIRE(board[4].playerName == "Guest_1");
}

TEST_CASE("TC-LB-01: date_achieved is written as YYYY-MM-DD (D4)",
          "[leaderboard][TC-LB-01]") {
    // SRS Appendix B and SAD 4.3.5: date_achieved is a 10-character date.
    TempDir dir;
    REQUIRE(LeaderboardRepository(dir.board())
                .addScore("Tejas", 120, GameMode::Timed, Difficulty::Hard)
                .status == ScoreSubmissionStatus::Submitted);
    const auto document = nlohmann::json::parse(readBytes(dir.board()));
    REQUIRE(isIsoDate(document.at("entries").at(0).at("date_achieved").get<std::string>()));
}

TEST_CASE("TC-LB-03: only scores inside the Top 10 are inserted", "[leaderboard][TC-LB-03]") {
    const std::vector<int> full{1000, 900, 800, 700, 600, 500, 400, 300, 200, 100};

    struct Case {
        int score;
        bool inserted;
    };
    // Each submission on a fresh full board (100, 200 ... 1000).
    for (const Case submission : {Case{1001, true}, Case{101, true}, Case{100, false},
                                  Case{99, false}}) {
        CAPTURE(submission.score);
        TempDir dir;
        writeBytes(dir.board(), boardJson(full));
        const std::string before = readBytes(dir.board());

        LeaderboardRepository repository(dir.board());
        const auto result =
            repository.addScore("New", submission.score, GameMode::Classic, Difficulty::Medium);
        const auto board = repository.top10();
        REQUIRE(board.size() == 10);
        if (submission.inserted) {
            REQUIRE(result.status == ScoreSubmissionStatus::Submitted);
            // The lowest entry (100) was dropped.
            REQUIRE(board.back().score == (submission.score == 101 ? 101 : 200));
            REQUIRE(std::any_of(board.begin(), board.end(), [&](const LeaderboardEntry& entry) {
                return entry.playerName == "New" && entry.score == submission.score;
            }));
        } else {
            // A tie with 10th place is not enough; the file stays as it was.
            REQUIRE(result.status == ScoreSubmissionStatus::NotQualified);
            REQUIRE(holds(dir.board(), before));
        }
    }

    // A board with 3 entries takes even a score of 1.
    TempDir dir;
    writeBytes(dir.board(), boardJson({300, 200, 100}));
    LeaderboardRepository repository(dir.board());
    const auto result = repository.addScore("Low", 1, GameMode::Classic, Difficulty::Medium);
    REQUIRE(result.status == ScoreSubmissionStatus::Submitted);
    REQUIRE(result.rank == std::size_t{4});
    REQUIRE(repository.top10().size() == 4);
}

TEST_CASE("TC-LB-04: missing, empty and truncated files show an empty board and get replaced",
          "[leaderboard][TC-LB-04]") {
    const char* const recoverable[] = {"missing", "01-empty-file.json", "03-truncated.json"};
    for (const char* name : recoverable) {
        CAPTURE(name);
        TempDir dir;
        if (std::string_view(name) != "missing") {
            install(CorpusDir / "leaderboard" / "malformed" / name, dir.board());
        }
        LeaderboardRepository repository(dir.board());
        REQUIRE(repository.top10().empty());

        // A valid result is then stored and the file is valid again.
        const auto added = repository.addScore("Tejas", 120, GameMode::Classic, Difficulty::Easy);
        REQUIRE(added.status == ScoreSubmissionStatus::Submitted);
        REQUIRE(SecureFileStore(dir.board()).readJson().status == StoreReadStatus::Ok);
        REQUIRE(repository.top10().size() == 1);
    }
}

TEST_CASE("Known defect D6: an empty or whitespace-only board shows no warning",
          "[leaderboard][TC-LB-04][known-defect][!shouldfail]") {
    // STP TC-LB-04 lists "empty" with the other corrupt fixtures: "empty
    // leaderboard shown; warning logged". Today an empty or blank file is
    // read as a new board, with no warning (truncated files do warn).
    for (const char* name : {"01-empty-file.json", "02-whitespace-only.json"}) {
        CAPTURE(name);
        TempDir dir;
        install(CorpusDir / "leaderboard" / "malformed" / name, dir.board());
        std::string warning;
        REQUIRE(LeaderboardRepository(dir.board()).top10(&warning).empty());
        CHECK_FALSE(warning.empty());
    }
}

TEST_CASE("TC-LB-04: a schema-invalid board is replaced by the next valid score (D3)",
          "[leaderboard][TC-LB-04]") {
    // STP TC-LB-04 step 2: "valid result then stored (bad file replaced
    // atomically)" for wrong type, negative score and bad enum.
    const char* const fixturesToReplace[] = {"14-score-as-string.json", "15-negative-score.json",
                                             "18-unknown-game-mode.json"};
    for (const char* name : fixturesToReplace) {
        CAPTURE(name);
        TempDir dir;
        install(CorpusDir / "leaderboard" / "malformed" / name, dir.board());
        LeaderboardRepository repository(dir.board());
        CHECK(repository.addScore("Tejas", 120, GameMode::Classic, Difficulty::Easy).status ==
              ScoreSubmissionStatus::Submitted);
    }
}

// ---------------------------------------------------------------------------
// TC-SEC-01, TC-SEC-05, TC-SEC-06: names and untrusted strings
// ---------------------------------------------------------------------------

TEST_CASE("TC-SEC-01: unsafe names are rejected and nothing unsafe is written",
          "[security][TC-SEC-01]") {
    using namespace std::string_literals;
    const std::string rejected[] = {
        "Ab,cd", "a\"b", "a\nb", "a\tb", "{\"x\":1}", "../x", "a\0b"s,
        "emoji \xF0\x9F\x98\x80", "Espa\xC3\xB1" "a", "", "   ", "C:\\x", "a;b", "a|b",
    };
    TempDir dir;
    LeaderboardRepository repository(dir.board());
    for (const std::string& name : rejected) {
        CAPTURE(name);
        REQUIRE_FALSE(snake::isValidPlayerName(name));
        REQUIRE(repository.addScore(name, 50, GameMode::Classic, Difficulty::Easy).status ==
                ScoreSubmissionStatus::Error);
        REQUIRE_FALSE(fs::exists(dir.board())); // nothing written
    }

    // The valid example is accepted and stored as typed.
    REQUIRE(snake::isValidPlayerName("A_b-1 x"));
    REQUIRE(repository.addScore("A_b-1 x", 50, GameMode::Classic, Difficulty::Easy).status ==
            ScoreSubmissionStatus::Submitted);
    const auto document = nlohmann::json::parse(readBytes(dir.board()));
    REQUIRE(document.at("entries").at(0).at("player_name") == "A_b-1 x");
}

TEST_CASE("TC-SEC-05: hostile strings in a hand-edited file are data, never code",
          "[security][TC-SEC-05]") {
    const char* const hostile[] = {"%s%n", "$(calc)", "<script>", "${PATH}"};
    for (const char* name : hostile) {
        CAPTURE(name);
        TempDir dir;
        nlohmann::json document = nlohmann::json::parse(boardJson({300, 200}));
        document["entries"][0]["player_name"] = name;
        writeBytes(dir.board(), document.dump(2));

        // The entry is rejected (characters outside [A-Za-z0-9 _-]), so the
        // whole board falls back to empty; nothing is run or formatted.
        REQUIRE(LeaderboardRepository(dir.board()).top10().empty());
    }

    // In fields the schema ignores, hostile text stays inert data.
    TempDir dir;
    nlohmann::json document = nlohmann::json::parse(boardJson({300}));
    document["note"] = "%s%n $(calc) <script> ${PATH}";
    writeBytes(dir.board(), document.dump(2));
    const auto board = LeaderboardRepository(dir.board()).top10();
    REQUIRE(board.size() == 1);
    REQUIRE(board[0].playerName == "P1");
}

TEST_CASE("TC-SEC-06: a 10,000-character name never gets past 15 characters",
          "[security][TC-SEC-06]") {
    const std::string huge(10000, 'A');
    REQUIRE_FALSE(snake::isValidPlayerName(huge));
    REQUIRE(snake::sanitizePlayerName(huge).size() == 15);

    std::string mixed;
    for (int i = 0; i < 10000; ++i) {
        mixed += (i % 7 == 0) ? '%' : static_cast<char>('a' + i % 26);
    }
    REQUIRE(snake::sanitizePlayerName(mixed).size() <= 15);

    TempDir dir;
    LeaderboardRepository repository(dir.board());
    REQUIRE(repository.addScore(huge, 50, GameMode::Classic, Difficulty::Easy).status ==
            ScoreSubmissionStatus::Error);
    REQUIRE_FALSE(fs::exists(dir.board()));
}

// ---------------------------------------------------------------------------
// TC-SEC-04: what a crash at each step of the safe write leaves behind
// ---------------------------------------------------------------------------

TEST_CASE("TC-SEC-04 (simulated leftovers): the files a crash leaves still load the old or new board",
          "[security][TC-SEC-04]") {
    const std::string oldBoard = boardJson({300, 200, 100});
    const std::string newBoard = boardJson({400, 300, 200, 100});

    // Simulated: these files are written by the test, no save is crashed.
    // tests/SecureWriteFaultTests.cpp crashes the real save (Linux).
    // What a crash leaves on disk at each point (a)-(c): the old file plus a
    // temporary sibling, as SecureFileStore::writeJson names it.
    const std::string leftovers[] = {
        "",                            // (a) after the temporary file was opened
        newBoard.substr(0, 40),        // (b) part-way through writing it
        newBoard,                      // (c) after it was written and flushed
    };
    for (const std::string& temporary : leftovers) {
        CAPTURE(temporary.size());
        TempDir dir;
        writeBytes(dir.board(), oldBoard);
        writeBytes(dir.path() / "leaderboard.json.tmp.12345", temporary);
        const auto board = LeaderboardRepository(dir.board()).top10();
        REQUIRE(board.size() == 3); // the old, valid board
        REQUIRE(board[0].score == 300);
    }

    // (d) after the rename: the new, valid board.
    TempDir dir;
    writeBytes(dir.board(), oldBoard);
    LeaderboardRepository repository(dir.board());
    REQUIRE(repository.addScore("New", 400, GameMode::Classic, Difficulty::Easy).status ==
            ScoreSubmissionStatus::Submitted);
    REQUIRE(repository.top10().size() == 4);
    REQUIRE(repository.top10()[0].score == 400);
    // No temporary file is left after a successful save.
    for (const auto& item : fs::directory_iterator(dir.path())) {
        CAPTURE(item.path().filename().string());
        REQUIRE(item.path().filename() == "leaderboard.json");
    }
}

TEST_CASE("TC-SEC-04 (simulated leftover): a temporary file older than 24 hours is removed (D5)",
          "[security][TC-SEC-04]") {
    // STP TC-SEC-04: "stale .tmp removed" (SAD 4.3.4 removeStaleTemp). The
    // agreed policy (#48) only removes leftovers older than 24 hours, so the
    // leftover is made 48 hours old. A fresh one is kept on purpose: see the
    // "stale temp safety" test below.
    TempDir dir;
    writeBytes(dir.board(), boardJson({300}));
    const fs::path stale = dir.path() / "leaderboard.json.tmp.12345";
    writeBytes(stale, "{\"version\": 1, \"ent");
    age(stale, 48h);
    LeaderboardRepository repository(dir.board());
    (void)repository.top10();
    REQUIRE(repository.addScore("New", 400, GameMode::Classic, Difficulty::Easy).status ==
            ScoreSubmissionStatus::Submitted);
    REQUIRE_FALSE(fs::exists(stale));
}

TEST_CASE("TC-SEC-04 (simulated orphan lock): a lock sidecar older than 24 hours without its temporary file is removed (D5)",
          "[security][TC-SEC-04]") {
    // A crash before the temporary file is created, or after the rename,
    // leaves only "<board>.tmp.<nonce>.lock" (#48 review). Under the agreed
    // policy, the next load removes it once it is older than 24 hours.
    TempDir dir;
    writeBytes(dir.board(), boardJson({300}));
    const fs::path orphan = dir.path() / "leaderboard.json.tmp.991.lock";
    writeBytes(orphan, "");
    age(orphan, 48h);
    REQUIRE(LeaderboardRepository(dir.board()).top10().size() == 1);
    REQUIRE_FALSE(fs::exists(orphan));
}

TEST_CASE("TC-SEC-04 (stale temp safety): loads and saves remove nothing but stale, unlocked leftovers of this board",
          "[security][TC-SEC-04]") {
    // Guards the D5 fix and the agreed policy (#48): only regular files named
    // exactly "<board>.tmp.<decimal nonce>", or that name plus ".lock" once
    // its temporary file is gone, older than 24 hours and not locked by a live
    // save, may be removed. Everything below must survive a load and a save.
    TempDir dir;
    writeBytes(dir.board(), boardJson({300}));
    const std::string keep = "keep me";

    struct Kept {
        fs::path path;
        bool aged;
    };
    const Kept files[] = {
        {dir.path() / "leaderboard.json.bak", true},
        {dir.path() / "leaderboard.json.tmp", true},             // no nonce
        {dir.path() / "leaderboard.json.tmp.12ab", true},        // not a decimal nonce
        {dir.path() / "config.json.tmp.12345", true},            // the settings store's
        {dir.path() / "leaderboard.jsonx.tmp.12345", true},      // another file's
        {dir.path() / "notes.txt", true},
        {dir.path() / "leaderboard.json.tmp.4242", false},       // fresh: a save may be running
        {dir.path() / "leaderboard.json.tmp.4343.lock", false},  // fresh sidecar
    };
    for (const Kept& file : files) {
        writeBytes(file.path, keep);
        if (file.aged) {
            age(file.path, 48h);
        }
    }

    // A stale-looking temporary file whose save still holds its lock.
    const fs::path busy = dir.path() / "leaderboard.json.tmp.5151";
    const fs::path busyLock = dir.path() / "leaderboard.json.tmp.5151.lock";
    writeBytes(busy, keep);
    writeBytes(busyLock, "");
    age(busy, 48h);
    age(busyLock, 48h);
    const HeldLock lock(busyLock);
    REQUIRE(lock.held());

    // A directory with a temporary file's name.
    const fs::path folder = dir.path() / "leaderboard.json.tmp.777";
    fs::create_directory(folder);
    writeBytes(folder / "inside.txt", keep);
    std::error_code ignored; // directory times cannot be set everywhere
    fs::last_write_time(folder, fs::file_time_type::clock::now() - 48h, ignored);

    const fs::path outside = dir.path() / "outside.json";
    writeBytes(outside, keep);
#if !defined(_WIN32)
    // Old symbolic links with a temporary file's or a sidecar's name.
    const fs::path link = dir.path() / "leaderboard.json.tmp.888";
    const fs::path lockLink = dir.path() / "leaderboard.json.tmp.889.lock";
    fs::create_symlink(outside, link);
    fs::create_symlink(outside, lockLink);
    REQUIRE(ageLink(link, 48h));
    REQUIRE(ageLink(lockLink, 48h));
#endif

    LeaderboardRepository repository(dir.board());
    (void)repository.top10();
    REQUIRE(repository.addScore("New", 400, GameMode::Classic, Difficulty::Easy).status ==
            ScoreSubmissionStatus::Submitted);
    (void)LeaderboardRepository(dir.board()).top10();

    for (const Kept& file : files) {
        CAPTURE(file.path.filename().string());
        REQUIRE(holds(file.path, keep));
    }
    REQUIRE(holds(busy, keep));
    REQUIRE(fs::exists(busyLock));
    REQUIRE(fs::is_directory(folder));
    REQUIRE(holds(folder / "inside.txt", keep));
    REQUIRE(holds(outside, keep));
#if !defined(_WIN32)
    REQUIRE(fs::is_symlink(fs::symlink_status(link)));
    REQUIRE(fs::is_symlink(fs::symlink_status(lockLink)));
#endif
    REQUIRE(LeaderboardRepository(dir.board()).top10().size() == 2);
}

TEST_CASE("TC-SEC-04 (real refusal): a save into a read-only folder or locked file keeps the old board",
          "[security][TC-SEC-04]") {
    TempDir dir;
    const std::string oldBoard = boardJson({300, 200, 100});
    writeBytes(dir.board(), oldBoard);

#if defined(_WIN32)
    // Locked target: another handle holds the file without sharing.
    int handle = -1;
    REQUIRE(_wsopen_s(&handle, dir.board().c_str(), _O_RDONLY | _O_BINARY, _SH_DENYRW,
                      _S_IREAD) == 0);
    const auto result =
        LeaderboardRepository(dir.board()).addScore("New", 400, GameMode::Classic, Difficulty::Easy);
    _close(handle);
#else
    if (::geteuid() == 0) {
        SKIP("Root ignores directory permissions; run as a normal user (as CI does)");
    }
    // A directory that cannot be written: neither temp file nor rename works.
    fs::permissions(dir.path(), fs::perms::owner_read | fs::perms::owner_exec,
                    fs::perm_options::replace);
    const auto result =
        LeaderboardRepository(dir.board()).addScore("New", 400, GameMode::Classic, Difficulty::Easy);
    fs::permissions(dir.path(), fs::perms::owner_all, fs::perm_options::add);
#endif
    REQUIRE(result.status == ScoreSubmissionStatus::Error);
    REQUIRE(holds(dir.board(), oldBoard));
}

// ---------------------------------------------------------------------------
// Evidence for the execution report (hidden; run with "[.evidence]")
// ---------------------------------------------------------------------------

TEST_CASE("Evidence: per-file results of the leaderboard corpus", "[.evidence]") {
    const char* const statusNames[] = {"Ok", "NotFound", "EmptyFile", "TooLarge",
                                       "InvalidJson", "UnsafeJson", "UnsafeFile", "IoError"};
    for (const std::string set : {"valid", "malformed"}) {
        for (const fs::path& fixture : fixtures(set)) {
            TempDir dir;
            install(fixture, dir.board());
            const auto read = SecureFileStore(dir.board()).readJson();
            const Timed loaded = timedTop10(dir.board());
            std::cout << set << " | " << fixture.filename().string() << " | "
                      << fs::file_size(fixture) << " B | "
                      << statusNames[static_cast<int>(read.status)] << " | "
                      << loaded.entries.size() << " entries | " << loaded.took.count()
                      << " ms\n";
        }
    }
    SUCCEED();
}
