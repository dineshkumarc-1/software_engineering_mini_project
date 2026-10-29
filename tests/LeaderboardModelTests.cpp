#include "snakextreme/LeaderboardModel.hpp"
#include "snakextreme/LeaderboardTable.hpp"
#include <limits>
#include <catch2/catch_test_macros.hpp>
#include <chrono>
#include <filesystem>
#include <fstream>

namespace {
struct Fixture {
    std::filesystem::path directory = std::filesystem::temp_directory_path() /
        ("snake-model-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    Fixture() { std::filesystem::create_directory(directory); }
    ~Fixture() { std::error_code ec; std::filesystem::remove_all(directory, ec); }
};
}
TEST_CASE("Completed sessions persist once, refresh and survive restart", "[leaderboard][integration][TC-LB-02]") {
    Fixture fixture;
    const auto path = fixture.directory / "leaderboard.json";
    snake::LeaderboardModel model(path);
    model.beginSession();
    model.finish(" Dinesh ", 120, snake::GameMode::Timed, snake::Difficulty::Hard);
    REQUIRE(model.message().find("Score saved: rank 1") == 0);
    model.finish("Duplicate", 999, snake::GameMode::Classic, snake::Difficulty::Easy);
    model.refresh();
    REQUIRE(model.entries().size() == 1);
    REQUIRE(model.entries()[0].playerName == "Dinesh");
    model.beginSession();
    model.finish("Ayushi", 240, snake::GameMode::Obstacle, snake::Difficulty::Medium);
    snake::LeaderboardModel restarted(path);
    restarted.refresh();
    REQUIRE(restarted.entries().size() == 2);
    REQUIRE(restarted.entries()[0].playerName == "Ayushi");
    REQUIRE(restarted.entries()[1].score == 120);
}
TEST_CASE("Leaderboard adapter safely handles missing corrupt and unsafe files", "[leaderboard][integration]") {
    Fixture fixture;
    const auto path = fixture.directory / "leaderboard.json";
    snake::LeaderboardModel model(path);
    model.refresh();
    REQUIRE(model.entries().empty());
    REQUIRE(model.message().empty());
    { std::ofstream file(path); file << "{broken"; }
    model.refresh();
    REQUIRE(model.entries().empty());
    REQUIRE_FALSE(model.message().empty());
    std::filesystem::remove(path);
    std::filesystem::create_directory(path);
    model.beginSession();
    model.finish("Dinesh", 120, snake::GameMode::Classic, snake::Difficulty::Easy);
    REQUIRE(model.message() == "E_LEADERBOARD_SAVE: score not saved");
    REQUIRE(std::filesystem::is_directory(path));
}

TEST_CASE("Leaderboard rows expose all six STP fields for every mode and difficulty",
          "[leaderboard][TC-LB-02][D7]") {
    std::size_t rank = 1U;
    for (const auto mode : snake::AllGameModes) {
        for (const auto difficulty : snake::AllDifficulties) {
            const snake::LeaderboardEntry entry{"WWWWWWWWWWWWWWW",
                std::numeric_limits<int>::max(), mode, difficulty, "2024-02-29"};
            const auto row = snake::leaderboardRow(rank, entry);
            REQUIRE(row.size() == 6U);
            CHECK(row[0] == std::to_string(rank));
            CHECK(row[1] == entry.playerName);
            CHECK(row[2] == "2147483647");
            CHECK(row[3] == snake::toString(mode));
            CHECK(row[4] == snake::toString(difficulty));
            CHECK(row[5] == "2024-02-29");
            ++rank;
        }
    }
}
TEST_CASE("Leaderboard table column bounds are disjoint inside the navigation view",
          "[leaderboard][TC-LB-02][D7]") {
    const auto& columns = snake::LeaderboardColumns;
    REQUIRE(columns.size() == 6U);
    CHECK(columns[0].heading == "Rank");
    CHECK(columns[1].heading == "Player name");
    CHECK(columns[2].heading == "Score");
    CHECK(columns[3].heading == "Mode");
    CHECK(columns[4].heading == "Difficulty");
    CHECK(columns[5].heading == "Date");
    for (std::size_t i = 0; i < columns.size(); ++i) {
        CHECK(columns[i].left >= 0.f);
        CHECK(columns[i].width > 0.f);
        CHECK(columns[i].left + columns[i].width <= 900.f);
        if (i + 1U < columns.size()) {
            CHECK(columns[i].left + columns[i].width < columns[i + 1U].left);
        }
    }
}

TEST_CASE("Leaderboard fallback keeps the existing QA rank-name-score delimiter",
          "[leaderboard][TC-LB-02][D7]") {
    const snake::LeaderboardEntry entry{"Low 10", 10, snake::GameMode::Timed,
        snake::Difficulty::Medium, "2024-02-29"};
    CHECK(snake::leaderboardRowTitle(3U, entry) ==
          " | 3. Low 10 10 | Timed / Medium / 2024-02-29");
}
