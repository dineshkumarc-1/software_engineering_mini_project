#include "snakextreme/LeaderboardRepository.hpp"

#include <catch2/catch_test_macros.hpp>

#include <nlohmann/json.hpp>

#include <atomic>
#include <cstdint>
#include <limits>
#include <filesystem>
#include <fstream>
#include <string>
#include <iterator>
#include <vector>

namespace {

class TemporaryDirectory {
public:
    TemporaryDirectory() {
        static std::atomic<unsigned int> counter{0};

        path_ = std::filesystem::temp_directory_path() /
            ("snakextreme_leaderboard_test_" +
             std::to_string(counter.fetch_add(1)));

        std::filesystem::create_directories(path_);
    }

    ~TemporaryDirectory() {
        std::error_code ignored;
        std::filesystem::remove_all(path_, ignored);
    }

    [[nodiscard]]
    const std::filesystem::path& path() const noexcept {
        return path_;
    }

private:
    std::filesystem::path path_;
};

} // namespace

TEST_CASE(
    "A missing leaderboard starts empty",
    "[persistence][leaderboard]") {

    TemporaryDirectory directory;

    snake::LeaderboardRepository repository(
        directory.path() / "leaderboard.json");

    CHECK(repository.top10().empty());
}

TEST_CASE(
    "Leaderboard rejects invalid names and negative scores",
    "[persistence][leaderboard]") {

    TemporaryDirectory directory;

    snake::LeaderboardRepository repository(
        directory.path() / "leaderboard.json");

    CHECK(
    repository.addScore(
        "", 10,
        snake::GameMode::Classic,
        snake::Difficulty::Medium).status
    == snake::ScoreSubmissionStatus::Error);

CHECK(
    repository.addScore(
        "   ", 10,
        snake::GameMode::Classic,
        snake::Difficulty::Medium).status
    == snake::ScoreSubmissionStatus::Error);

CHECK(
    repository.addScore(
        "Alice@", 10,
        snake::GameMode::Classic,
        snake::Difficulty::Medium).status
    == snake::ScoreSubmissionStatus::Error);

CHECK(
    repository.addScore(
        "Alice", -1,
        snake::GameMode::Classic,
        snake::Difficulty::Medium).status
    == snake::ScoreSubmissionStatus::Error);

    CHECK(repository.top10().empty());
}

TEST_CASE(
    "Leaderboard trims accepted names and persists scores",
    "[persistence][leaderboard]") {

    TemporaryDirectory directory;

    const auto file =
        directory.path() / "leaderboard.json";

    snake::LeaderboardRepository repository(file);

    const auto result = repository.addScore(
    "  Alice  ",
    35,
    snake::GameMode::Classic,
    snake::Difficulty::Medium);

REQUIRE(result.status == snake::ScoreSubmissionStatus::Submitted);
REQUIRE(result.rank.has_value());
CHECK(*result.rank == 1U);

    const auto entries = repository.top10();

    REQUIRE(entries.size() == 1U);

    CHECK(entries[0].playerName == "Alice");
    CHECK(entries[0].score == 35);
    CHECK(entries[0].dateAchieved.size() == 10U);
    CHECK(entries[0].dateAchieved[4] == '-');
    CHECK(entries[0].dateAchieved[7] == '-');
    CHECK(repository.top10().size() == 1U);
}

TEST_CASE(
    "Leaderboard sorts descending and retains only ten entries",
    "[persistence][leaderboard]") {

    TemporaryDirectory directory;

    snake::LeaderboardRepository repository(
        directory.path() / "leaderboard.json");

    for (int score = 1; score <= 12; ++score) {
       const auto result = repository.addScore(
    "Player" + std::to_string(score),
    score,
    snake::GameMode::Classic,
    snake::Difficulty::Medium);

REQUIRE(
    result.status ==
    snake::ScoreSubmissionStatus::Submitted);
    }

    const auto entries = repository.top10();

    REQUIRE(entries.size() == 10U);

    CHECK(entries.front().score == 12);
    CHECK(entries.back().score == 3);

    for (std::size_t i = 1; i < entries.size(); ++i) {
        CHECK(
            entries[i - 1].score >=
            entries[i].score);
    }
}

TEST_CASE(
    "Leaderboard rejects a score tied with tenth place without rewriting the file",
    "[persistence][leaderboard]") {

    TemporaryDirectory directory;

    const auto file =
        directory.path() / "leaderboard.json";

    snake::LeaderboardRepository repository(file);

    // Fill the leaderboard with ten entries.
    for (int score = 1; score <= 10; ++score) {
        const auto result = repository.addScore(
            "Player" + std::to_string(score),
            score,
            snake::GameMode::Classic,
            snake::Difficulty::Medium);

        REQUIRE(
            result.status ==
            snake::ScoreSubmissionStatus::Submitted);
    }

    // Save the exact file contents before the tied submission.
    std::ifstream beforeInput(file, std::ios::binary);
    REQUIRE(beforeInput.good());

    const std::string contentsBefore(
        (std::istreambuf_iterator<char>(beforeInput)),
        std::istreambuf_iterator<char>());

    // Tenth place has 1 point. A tie must not qualify.
    const auto result = repository.addScore(
        "TiedPlayer",
        1,
        snake::GameMode::Classic,
        snake::Difficulty::Medium);

    CHECK(
        result.status ==
        snake::ScoreSubmissionStatus::NotQualified);

    CHECK_FALSE(result.rank.has_value());

    // Confirm that the file was not rewritten.
    std::ifstream afterInput(file, std::ios::binary);
    REQUIRE(afterInput.good());

    const std::string contentsAfter(
        (std::istreambuf_iterator<char>(afterInput)),
        std::istreambuf_iterator<char>());

    CHECK(contentsAfter == contentsBefore);
        // A score below tenth place must also be rejected.
    const auto belowResult = repository.addScore(
        "BelowPlayer",
        0,
        snake::GameMode::Classic,
        snake::Difficulty::Medium);

    CHECK(
        belowResult.status ==
        snake::ScoreSubmissionStatus::NotQualified);
    CHECK_FALSE(belowResult.rank.has_value());

    // A below-qualifying score must not rewrite the file either.
    std::ifstream afterBelowInput(file, std::ios::binary);
    REQUIRE(afterBelowInput.good());

    const std::string contentsAfterBelow(
        (std::istreambuf_iterator<char>(afterBelowInput)),
        std::istreambuf_iterator<char>());

    CHECK(contentsAfterBelow == contentsBefore);

    // The leaderboard must still contain exactly ten entries.
    const auto entries = repository.top10();

    REQUIRE(entries.size() == 10U);
    CHECK(entries.front().score == 10);
    CHECK(entries.back().score == 1);
}



TEST_CASE(
    "Leaderboard recovers from corrupt JSON on valid submission",
    "[persistence][leaderboard]") {

    TemporaryDirectory directory;

    const auto file =
        directory.path() / "leaderboard.json";

    {
        std::ofstream output(file, std::ios::binary);
        REQUIRE(output.good());
        output << "{ not valid JSON";
    }

    snake::LeaderboardRepository repository(file);

    // Displaying the damaged leaderboard should be safe.
    CHECK(repository.top10().empty());

    // A valid submission should recover the corrupted file.
    const auto result = repository.addScore(
        "Alice",
        10,
        snake::GameMode::Classic,
        snake::Difficulty::Medium);

    REQUIRE(
        result.status ==
        snake::ScoreSubmissionStatus::Submitted);

    REQUIRE(result.rank.has_value());
    CHECK(*result.rank == 1U);

    // The successful recovery must report a warning.
    CHECK_FALSE(result.warning.empty());

    // Confirm the replacement file can be read normally.
    const auto entries = repository.top10();

    REQUIRE(entries.size() == 1U);
    CHECK(entries[0].playerName == "Alice");
    CHECK(entries[0].score == 10);

    // Confirm the corrupted text was replaced with JSON
    // in the expected schema format.
    std::ifstream input(file, std::ios::binary);
    REQUIRE(input.good());

    const std::string contents(
        (std::istreambuf_iterator<char>(input)),
        std::istreambuf_iterator<char>());

    CHECK(contents != "{ not valid JSON");
    CHECK(contents.find("\"version\"") != std::string::npos);
    CHECK(contents.find("\"player_name\"") != std::string::npos);
}

TEST_CASE(
    "Leaderboard recovers from schema-invalid JSON on the next valid submission",
    "[persistence][leaderboard]") {

    TemporaryDirectory directory;
    const auto file = directory.path() / "leaderboard.json";

    const std::vector<std::string> invalidBoards = {
        R"({"version":1,"entries":[{"player_name":"Alice","score":"100","game_mode":"Classic","difficulty":"Medium","date_achieved":"2026-10-29"}]})",
        R"({"version":1,"entries":[{"player_name":"Alice","score":-1,"game_mode":"Classic","difficulty":"Medium","date_achieved":"2026-10-29"}]})",
        R"({"version":1,"entries":[{"player_name":"Alice","score":100,"game_mode":"Unknown","difficulty":"Medium","date_achieved":"2026-10-29"}]})",
        R"({"version":1,"entries":[{"player_name":"Alice","score":100,"game_mode":"Classic","difficulty":"Medium","date_achieved":"28/10/2026"}]})"
    };

    for (const auto& invalidBoard : invalidBoards) {
        {
            std::ofstream output(file, std::ios::binary | std::ios::trunc);
            REQUIRE(output.good());
            output << invalidBoard;
        }

        snake::LeaderboardRepository repository(file);
        CHECK(repository.top10().empty());

        const auto result = repository.addScore(
            "Bob",
            200,
            snake::GameMode::Classic,
            snake::Difficulty::Medium);

        REQUIRE(result.status == snake::ScoreSubmissionStatus::Submitted);
        CHECK_FALSE(result.warning.empty());

        const auto entries = repository.top10();
        REQUIRE(entries.size() == 1U);
        CHECK(entries[0].playerName == "Bob");
        CHECK(entries[0].score == 200);

        std::ifstream input(file, std::ios::binary);
        REQUIRE(input.good());
        const std::string contents(
            (std::istreambuf_iterator<char>(input)),
            std::istreambuf_iterator<char>());

        CHECK(contents != invalidBoard);
        CHECK(contents.find("\"version\"") != std::string::npos);
        CHECK(contents.find("\"date_achieved\"") != std::string::npos);
    }
}

TEST_CASE(
    "Leaderboard rejects an on-disk board containing more than ten entries",
    "[persistence][leaderboard]") {

    TemporaryDirectory directory;
    const auto file = directory.path() / "leaderboard.json";

    nlohmann::json document;
    document["version"] = snake::LeaderboardSchemaVersion;
    document["entries"] = nlohmann::json::array();

    for (int score = 1; score <= 11; ++score) {
        document["entries"].push_back({
            {"player_name", "Player" + std::to_string(score)},
            {"score", score},
            {"game_mode", "Classic"},
            {"difficulty", "Medium"},
            {"date_achieved", "2026-10-29"}
        });
    }

    {
        std::ofstream output(file, std::ios::binary);
        REQUIRE(output.good());
        output << document.dump(2);
    }

    snake::LeaderboardRepository repository(file);
    CHECK(repository.top10().empty());

    const auto result = repository.addScore(
        "Recovery",
        100,
        snake::GameMode::Classic,
        snake::Difficulty::Medium);

    REQUIRE(result.status == snake::ScoreSubmissionStatus::Submitted);
    CHECK_FALSE(result.warning.empty());

    const auto entries = repository.top10();
    REQUIRE(entries.size() == 1U);
    CHECK(entries[0].playerName == "Recovery");
    CHECK(entries[0].score == 100);
}

TEST_CASE(
    "Leaderboard strictly validates YYYY-MM-DD calendar dates and leap years",
    "[persistence][leaderboard]") {

    TemporaryDirectory directory;
    const auto file = directory.path() / "leaderboard.json";

    const auto writeBoardWithDate = [&](const std::string& date) {
        nlohmann::json document;
        document["version"] = snake::LeaderboardSchemaVersion;
        document["entries"] = nlohmann::json::array({
            {
                {"player_name", "Alice"},
                {"score", 50},
                {"game_mode", "Classic"},
                {"difficulty", "Medium"},
                {"date_achieved", date}
            }
        });

        std::ofstream output(file, std::ios::binary | std::ios::trunc);
        REQUIRE(output.good());
        output << document.dump(2);
    };

    SECTION("accepts valid Gregorian dates and leap days") {
        const std::vector<std::string> validDates = {
            "0001-01-01",
            "9999-12-31",
            "2000-02-29",
            "2024-02-29",
            "2024-03-01"
        };

        for (const auto& date : validDates) {
            CAPTURE(date);
            writeBoardWithDate(date);
            CHECK(snake::LeaderboardRepository(file).top10().size() == 1U);
        }
    }

    SECTION("rejects invalid, impossible, or non-date formats") {
        const std::vector<std::string> invalidDates = {
            "0000-01-01",
            "1900-02-29",
            "2100-02-29",
            "2023-02-29",
            "2024-02-30",
            "2024-13-01",
            "2024-00-01",
            "2024/02/29",
            "2024-2-09",
            "2024-02-29T00:00:00Z",
            "28/10/2026",
            ""
        };

        for (const auto& date : invalidDates) {
            CAPTURE(date);
            writeBoardWithDate(date);
            CHECK(snake::LeaderboardRepository(file).top10().empty());
        }
    }
}

TEST_CASE(
    "Leaderboard does not overwrite an oversized on-disk file",
    "[persistence][leaderboard][security]") {

    TemporaryDirectory directory;
    const auto file = directory.path() / "leaderboard.json";
    const std::string original(
        snake::SecureFileStore::MaxFileSizeBytes + 1U, 'x');

    {
        std::ofstream output(file, std::ios::binary);
        REQUIRE(output.good());
        output.write(
            original.data(),
            static_cast<std::streamsize>(original.size()));
        REQUIRE(output.good());
    }

    snake::LeaderboardRepository repository(file);
    CHECK(repository.top10().empty());

    const auto result = repository.addScore(
        "Bob", 10, snake::GameMode::Classic, snake::Difficulty::Medium);
    CHECK(result.status == snake::ScoreSubmissionStatus::Error);
    CHECK(result.warning.empty());

    std::ifstream input(file, std::ios::binary);
    REQUIRE(input.good());
    const std::string after(
        (std::istreambuf_iterator<char>(input)),
        std::istreambuf_iterator<char>());
    CHECK(after == original);
}

TEST_CASE(
    "Leaderboard loads a valid specification-format fixture",
    "[persistence][leaderboard]") {

    TemporaryDirectory directory;

    const auto file =
        directory.path() / "leaderboard.json";

    {
        std::ofstream output(file, std::ios::binary);
        REQUIRE(output.good());

        output <<
            R"({
                "version": 1,
                "entries": [
                    {
                        "player_name": "Alice",
                        "score": 35,
                        "game_mode": "Classic",
                        "difficulty": "Medium",
                        "date_achieved": "2026-10-28"
                    }
                ]
            })";
    }

    snake::LeaderboardRepository repository(file);

    const auto entries = repository.top10();

    REQUIRE(entries.size() == 1U);

    CHECK(entries[0].playerName == "Alice");
    CHECK(entries[0].score == 35);
    CHECK(entries[0].gameMode == snake::GameMode::Classic);
    CHECK(entries[0].difficulty == snake::Difficulty::Medium);
    CHECK(
        entries[0].dateAchieved ==
        "2026-10-28");
}

TEST_CASE(
    "An empty leaderboard file accepts a new score",
    "[persistence][leaderboard]") {

    TemporaryDirectory directory;

    const auto file =
        directory.path() / "leaderboard.json";

    // Create a file that exists but contains no data.
    {
        std::ofstream output(file, std::ios::binary);
        REQUIRE(output.good());
    }

    snake::LeaderboardRepository repository(file);

    CHECK(repository.top10().empty());

    const auto result = repository.addScore(
    "Alice",
    25,
    snake::GameMode::Classic,
    snake::Difficulty::Medium);

REQUIRE(result.status == snake::ScoreSubmissionStatus::Submitted);
REQUIRE(result.rank.has_value());
CHECK(*result.rank == 1U);
CHECK(result.warning.empty()); // Empty-file submission retains existing behavior.
    const auto entries = repository.top10();

    REQUIRE(entries.size() == 1U);
    CHECK(entries[0].playerName == "Alice");
    CHECK(entries[0].score == 25);
}

TEST_CASE(
    "Leaderboard top10 optionally reports read fallback warnings",
    "[persistence][leaderboard]") {

    TemporaryDirectory directory;
    const auto file = directory.path() / "leaderboard.json";
    snake::LeaderboardRepository repository(file);

    std::string warning = "stale warning";
    CHECK(repository.top10(&warning).empty());
    CHECK(warning.empty()); // Missing file is a normal empty leaderboard.

    {
        std::ofstream output(file, std::ios::binary);
        REQUIRE(output.good());
        output << R"({"version":1,"entries":[{"player_name":"Alice","score":-1,"game_mode":"Classic","difficulty":"Medium","date_achieved":"2026-10-29"}]})";
    }

    warning.clear();
    CHECK(repository.top10(&warning).empty());
    CHECK_FALSE(warning.empty());
    CHECK(warning.find("E_LEADERBOARD_READ") != std::string::npos);

    {
        std::ofstream output(file, std::ios::binary | std::ios::trunc);
        REQUIRE(output.good());
        output << R"({"version":1,"entries":[{"player_name":"Alice","score":50,"game_mode":"Classic","difficulty":"Medium","date_achieved":"2026-10-29"}]})";
    }

    warning = "stale warning";
    const auto entries = repository.top10(&warning);
    REQUIRE(entries.size() == 1U);
    CHECK(warning.empty());

    // Existing callers that omit the optional argument remain source-compatible.
    CHECK(repository.top10().size() == 1U);
}


TEST_CASE(
    "A zero-byte leaderboard returns a warning without rewriting the file",
    "[persistence][leaderboard][warning]") {

    TemporaryDirectory directory;
    const auto file = directory.path() / "leaderboard.json";
    {
        std::ofstream output(file, std::ios::binary | std::ios::trunc);
        REQUIRE(output.good());
    }

    std::ifstream beforeInput(file, std::ios::binary);
    REQUIRE(beforeInput.good());
    const std::string contentsBefore(
        (std::istreambuf_iterator<char>(beforeInput)),
        std::istreambuf_iterator<char>());
    CHECK(contentsBefore.empty());

    const snake::LeaderboardRepository repository(file);
    std::string warning = "stale warning";
    CHECK(repository.top10(&warning).empty());
    CHECK(
        warning ==
        "E_LEADERBOARD_READ: invalid or unreadable board; empty table shown");

    CHECK(std::filesystem::exists(file));
    std::ifstream afterInput(file, std::ios::binary);
    REQUIRE(afterInput.good());
    const std::string contentsAfter(
        (std::istreambuf_iterator<char>(afterInput)),
        std::istreambuf_iterator<char>());
    CHECK(contentsAfter == contentsBefore);
}

TEST_CASE(
    "A whitespace-only leaderboard returns a warning without rewriting the file",
    "[persistence][leaderboard][warning]") {

    TemporaryDirectory directory;
    const auto file = directory.path() / "leaderboard.json";
    const std::string original = " \t\r\n  \n";
    {
        std::ofstream output(file, std::ios::binary | std::ios::trunc);
        REQUIRE(output.good());
        output << original;
        REQUIRE(output.good());
    }

    std::ifstream beforeInput(file, std::ios::binary);
    REQUIRE(beforeInput.good());
    const std::string contentsBefore(
        (std::istreambuf_iterator<char>(beforeInput)),
        std::istreambuf_iterator<char>());
    CHECK(contentsBefore == original);

    const snake::LeaderboardRepository repository(file);
    std::string warning = "stale warning";
    CHECK(repository.top10(&warning).empty());
    CHECK(
        warning ==
        "E_LEADERBOARD_READ: invalid or unreadable board; empty table shown");

    CHECK(std::filesystem::exists(file));
    std::ifstream afterInput(file, std::ios::binary);
    REQUIRE(afterInput.good());
    const std::string contentsAfter(
        (std::istreambuf_iterator<char>(afterInput)),
        std::istreambuf_iterator<char>());
    CHECK(contentsAfter == contentsBefore);
}

TEST_CASE(
    "A valid empty leaderboard does not warn or rewrite the file",
    "[persistence][leaderboard][warning]") {

    TemporaryDirectory directory;
    const auto file = directory.path() / "leaderboard.json";
    const std::string original =
        "{\n  \"version\": 1,\n  \"entries\": []\n}\n";
    {
        std::ofstream output(file, std::ios::binary | std::ios::trunc);
        REQUIRE(output.good());
        output << original;
        REQUIRE(output.good());
    }

    std::ifstream beforeInput(file, std::ios::binary);
    REQUIRE(beforeInput.good());
    const std::string contentsBefore(
        (std::istreambuf_iterator<char>(beforeInput)),
        std::istreambuf_iterator<char>());
    CHECK(contentsBefore == original);

    const snake::LeaderboardRepository repository(file);
    std::string warning = "stale warning";
    CHECK(repository.top10(&warning).empty());
    CHECK(warning.empty());

    std::ifstream afterInput(file, std::ios::binary);
    REQUIRE(afterInput.good());
    const std::string contentsAfter(
        (std::istreambuf_iterator<char>(afterInput)),
        std::istreambuf_iterator<char>());
    CHECK(contentsAfter == contentsBefore);
}


TEST_CASE(
    "Leaderboard rejects unsupported game modes and difficulty values",
    "[persistence][leaderboard][validation]") {

    TemporaryDirectory directory;
    const auto file = directory.path() / "leaderboard.json";
    snake::LeaderboardRepository repository(file);

    const auto invalidMode = repository.addScore(
        "Alice",
        10,
        static_cast<snake::GameMode>(999),
        snake::Difficulty::Medium);

    CHECK(invalidMode.status == snake::ScoreSubmissionStatus::Error);
    CHECK_FALSE(std::filesystem::exists(file));

    const auto invalidDifficulty = repository.addScore(
        "Alice",
        10,
        snake::GameMode::Classic,
        static_cast<snake::Difficulty>(999));

    CHECK(invalidDifficulty.status == snake::ScoreSubmissionStatus::Error);
    CHECK_FALSE(std::filesystem::exists(file));
}

TEST_CASE(
    "Equal leaderboard scores receive the correct rank and retain insertion order",
    "[persistence][leaderboard][ranking]") {

    TemporaryDirectory directory;
    snake::LeaderboardRepository repository(
        directory.path() / "leaderboard.json");

    const auto first = repository.addScore(
        "Alice", 100, snake::GameMode::Classic, snake::Difficulty::Medium);
    REQUIRE(first.status == snake::ScoreSubmissionStatus::Submitted);
    REQUIRE(first.rank.has_value());
    CHECK(*first.rank == 1U);

    const auto tied = repository.addScore(
        "Bob", 100, snake::GameMode::Timed, snake::Difficulty::Hard);
    REQUIRE(tied.status == snake::ScoreSubmissionStatus::Submitted);
    REQUIRE(tied.rank.has_value());
    CHECK(*tied.rank == 2U);

    auto entries = repository.top10();
    REQUIRE(entries.size() == 2U);
    CHECK(entries[0].playerName == "Alice");
    CHECK(entries[1].playerName == "Bob");
    CHECK(entries[0].score == entries[1].score);

    const auto higher = repository.addScore(
        "Charlie", 101, snake::GameMode::Obstacle, snake::Difficulty::Easy);
    REQUIRE(higher.status == snake::ScoreSubmissionStatus::Submitted);
    REQUIRE(higher.rank.has_value());
    CHECK(*higher.rank == 1U);

    entries = repository.top10();
    REQUIRE(entries.size() == 3U);
    CHECK(entries[0].playerName == "Charlie");
    CHECK(entries[1].playerName == "Alice");
    CHECK(entries[2].playerName == "Bob");
}

TEST_CASE(
    "Malformed leaderboard JSON reports a read warning without changing the file",
    "[persistence][leaderboard][warning]") {

    TemporaryDirectory directory;
    const auto file = directory.path() / "leaderboard.json";
    const std::string malformed = R"({"version":1,"entries":[)";
    {
        std::ofstream output(file, std::ios::binary | std::ios::trunc);
        REQUIRE(output.good());
        output << malformed;
        REQUIRE(output.good());
    }

    const snake::LeaderboardRepository repository(file);
    std::string warning = "stale warning";

    CHECK(repository.top10(&warning).empty());
    CHECK(warning.find("E_LEADERBOARD_READ") != std::string::npos);

    std::ifstream input(file, std::ios::binary);
    REQUIRE(input.good());
    const std::string after(
        (std::istreambuf_iterator<char>(input)),
        std::istreambuf_iterator<char>());
    CHECK(after == malformed);
}

TEST_CASE(
    "Leaderboard rejects unhandled schema shapes and numeric boundary values",
    "[persistence][leaderboard][schema]") {

    TemporaryDirectory directory;
    const auto file = directory.path() / "leaderboard.json";
    const snake::LeaderboardRepository repository(file);

    const auto validEntry = [] {
        return nlohmann::json{
            {"player_name", "Alice"},
            {"score", 50},
            {"game_mode", "Classic"},
            {"difficulty", "Medium"},
            {"date_achieved", "2026-10-29"}
        };
    };

    const auto makeBoard = [](nlohmann::json entries) {
        return nlohmann::json{
            {"version", snake::LeaderboardSchemaVersion},
            {"entries", entries}
        };
    };

    std::vector<nlohmann::json> invalidDocuments;

    // The root and schema envelope must have the expected JSON shape.
    invalidDocuments.push_back(nlohmann::json::array());
    invalidDocuments.push_back(nlohmann::json{
        {"entries", nlohmann::json::array()}
    });
    invalidDocuments.push_back(nlohmann::json{
        {"version", "1"},
        {"entries", nlohmann::json::array()}
    });
    invalidDocuments.push_back(nlohmann::json{
        {"version", std::numeric_limits<std::uint64_t>::max()},
        {"entries", nlohmann::json::array()}
    });
    invalidDocuments.push_back(nlohmann::json{
        {"version", snake::LeaderboardSchemaVersion}
    });
    invalidDocuments.push_back(nlohmann::json{
        {"version", snake::LeaderboardSchemaVersion},
        {"entries", nlohmann::json::object()}
    });

    // Entries must be objects containing every required schema field.
    invalidDocuments.push_back(makeBoard(
        nlohmann::json::array({"not-an-entry-object"})));

    for (const std::string field : {
             "player_name", "score", "game_mode",
             "difficulty", "date_achieved"}) {
        auto entry = validEntry();
        entry.erase(field);
        invalidDocuments.push_back(
            makeBoard(nlohmann::json::array({entry})));
    }

    // Text fields must remain strings; a padded or illegal name is not
    // canonical persisted data even though submission can normalize input.
    for (const std::string field : {
             "player_name", "game_mode", "difficulty", "date_achieved"}) {
        auto entry = validEntry();
        entry[field] = 42;
        invalidDocuments.push_back(
            makeBoard(nlohmann::json::array({entry})));
    }

    {
        auto entry = validEntry();
        entry["player_name"] = "  Alice  ";
        invalidDocuments.push_back(
            makeBoard(nlohmann::json::array({entry})));
    }
    {
        auto entry = validEntry();
        entry["player_name"] = "Alice@";
        invalidDocuments.push_back(
            makeBoard(nlohmann::json::array({entry})));
    }
    {
        auto entry = validEntry();
        entry["difficulty"] = "Unknown";
        invalidDocuments.push_back(
            makeBoard(nlohmann::json::array({entry})));
    }

    // Scores must be integral, fit in int, and convert safely to int64_t.
    {
        auto entry = validEntry();
        entry["score"] = 50.5;
        invalidDocuments.push_back(
            makeBoard(nlohmann::json::array({entry})));
    }
    {
        auto entry = validEntry();
        entry["score"] =
            static_cast<long long>(std::numeric_limits<int>::max()) + 1LL;
        invalidDocuments.push_back(
            makeBoard(nlohmann::json::array({entry})));
    }
    {
        auto entry = validEntry();
        entry["score"] = std::numeric_limits<std::uint64_t>::max();
        invalidDocuments.push_back(
            makeBoard(nlohmann::json::array({entry})));
    }

    for (std::size_t index = 0; index < invalidDocuments.size(); ++index) {
        CAPTURE(index);
        {
            std::ofstream output(file, std::ios::binary | std::ios::trunc);
            REQUIRE(output.good());
            output << invalidDocuments[index].dump();
            REQUIRE(output.good());
        }

        std::string warning;
        CHECK(repository.top10(&warning).empty());
        CHECK(warning.find("E_LEADERBOARD_READ") != std::string::npos);
    }
}
