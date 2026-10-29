#include "snakextreme/LeaderboardModel.hpp"
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
