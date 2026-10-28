#include "snakextreme/LeaderboardRepository.hpp"

#include <catch2/catch_test_macros.hpp>

#include <atomic>
#include <filesystem>
#include <fstream>
#include <string>
#include <iterator>

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

    CHECK_FALSE(repository.addScore("", 10));
    CHECK_FALSE(repository.addScore("   ", 10));
    CHECK_FALSE(repository.addScore("Alice@", 10));
    CHECK_FALSE(repository.addScore("Alice", -1));

    CHECK(repository.top10().empty());
}

TEST_CASE(
    "Leaderboard trims accepted names and persists scores",
    "[persistence][leaderboard]") {

    TemporaryDirectory directory;

    const auto file =
        directory.path() / "leaderboard.json";

    snake::LeaderboardRepository repository(file);

    REQUIRE(repository.addScore("  Alice  ", 35));

    const auto entries = repository.top10();

    REQUIRE(entries.size() == 1U);

    CHECK(entries[0].playerName == "Alice");
    CHECK(entries[0].score == 35);
}

TEST_CASE(
    "Leaderboard sorts descending and retains only ten entries",
    "[persistence][leaderboard]") {

    TemporaryDirectory directory;

    snake::LeaderboardRepository repository(
        directory.path() / "leaderboard.json");

    for (int score = 1; score <= 12; ++score) {
        REQUIRE(repository.addScore(
            "Player" + std::to_string(score),
            score));
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
    "Leaderboard falls back to empty on corrupt JSON",
    "[persistence][leaderboard]") {

    TemporaryDirectory directory;

    const auto file =
        directory.path() / "leaderboard.json";

    {
        std::ofstream output(file, std::ios::binary);
        output << "{ not valid JSON";
    }

    snake::LeaderboardRepository repository(file);

    CHECK(repository.top10().empty());

// Do not overwrite the corrupt file silently.
CHECK_FALSE(repository.addScore("Alice", 10));

std::ifstream input(file, std::ios::binary);
const std::string contents(
    (std::istreambuf_iterator<char>(input)),
    std::istreambuf_iterator<char>());

CHECK(contents == "{ not valid JSON");
}

TEST_CASE(
    "Leaderboard rejects an invalid stored schema",
    "[persistence][leaderboard]") {

    TemporaryDirectory directory;

    const auto file =
        directory.path() / "leaderboard.json";

    {
        std::ofstream output(file, std::ios::binary);

        output <<
            R"({"entries":[{"playerName":"Alice@","score":100}]})";
    }

    snake::LeaderboardRepository repository(file);

    CHECK(repository.top10().empty());

    // Invalid stored data must not be silently accepted
    // when adding another entry.
    CHECK_FALSE(repository.addScore("Bob", 10));
}