#include "snakextreme/PerformanceProbe.hpp"
#include "snakextreme/GameplayController.hpp"
#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>
#include <fstream>
#include <random>
#include <string>
using namespace snake;
namespace {
const ProbeContext Playing{"Gameplay", "Obstacle", "Hard"};
struct ReportFile {
    std::filesystem::path path = std::filesystem::temp_directory_path() /
        ("snake-probe-" + std::to_string(std::random_device{}()) + ".json");
    ~ReportFile() { std::error_code ignored; std::filesystem::remove(path, ignored); }
};
}
TEST_CASE("Probe options are opt-in and reject ambiguous arguments", "[probe]") {
    CHECK(parseProbeOptions({}).output.empty()); CHECK(parseProbeOptions({"--help"}).help);
    auto options = parseProbeOptions({"--perf-probe", "run.json", "--probe-seconds", "300"});
    CHECK(options.output == "run.json"); CHECK(options.seconds == 300);
    for (const auto& args : std::vector<std::vector<std::string>>{
        {"--bad"}, {"--perf-probe"}, {"--perf-probe", ""}, {"--probe-seconds", "1"},
        {"--perf-probe", "out", "--probe-seconds", "0"}, {"--perf-probe", "out", "--probe-seconds", "3601"},
        {"--perf-probe", "out", "--probe-seconds", "1.5"}, {"--perf-probe", "out", "--probe-seconds", "999999999999999"},
        {"--perf-probe", "a", "--perf-probe", "b"}, {"--perf-probe", "--help"},
        {"--perf-probe", "out", "--probe-seconds", "1", "--probe-seconds", "2"}})
        CHECK_THROWS_AS(parseProbeOptions(args), std::invalid_argument);
}
TEST_CASE("FPS windows are exact seconds and stalls create zero-frame windows", "[probe]") {
    PerformanceProbe p; p.frame(500000, Playing); p.frame(3700000, Playing);
    REQUIRE(p.firstFrameUs() == 500000); REQUIRE(p.windows().size() == 3);
    CHECK(p.windows()[0].startUs == 500000); CHECK(p.windows()[0].frames == 1);
    CHECK(p.windows()[1].startUs == 1500000); CHECK(p.windows()[1].frames == 0);
    CHECK(p.windows()[2].frames == 0);
    CHECK(p.windows()[0].uniform); CHECK(p.windows()[1].uniform);
}
TEST_CASE("Boundary frames belong to the next window and partial tails are excluded", "[probe]") {
    PerformanceProbe p;
    for (int second = 0; second < 4; ++second)
        for (int frame = 0; frame < 60; ++frame) p.frame(second * 1000000LL + frame * 1000000LL / 60, Playing);
    REQUIRE(p.windows().size() == 3);
    for (const auto& w : p.windows()) CHECK(w.frames == 60);
}
TEST_CASE("Mixed-context windows never masquerade as benchmark gameplay", "[probe]") {
    PerformanceProbe p; p.frame(0, Playing); p.frame(500000, {"Paused", "Obstacle", "Hard"});
    p.frame(1000000, {"Paused", "Obstacle", "Hard"}); p.frame(2000000, {"Paused", "Obstacle", "Hard"});
    REQUIRE(p.windows().size() == 2); CHECK_FALSE(p.windows()[0].uniform);
    CHECK(p.windows()[1].uniform); CHECK(p.windows()[1].context.screen == "Paused");
}
TEST_CASE("A restart splits otherwise identical gameplay contexts", "[probe]") {
    PerformanceProbe p; p.beginSession(); p.frame(0, Playing);
    p.beginSession(); p.frame(500000, Playing); p.frame(1000000, Playing); p.frame(2000000, Playing);
    REQUIRE(p.windows().size() == 2); CHECK_FALSE(p.windows()[0].uniform);
    CHECK(p.windows()[1].uniform); CHECK(p.windows()[1].session == 2);
}
TEST_CASE("Input registration is distinct from engine application and display", "[probe]") {
    PerformanceProbe p; p.beginSession(); p.input(1000, 1010, true, "Up"); p.input(1020, 1030, false, "Down");
    p.frame(1100, Playing); CHECK_FALSE(p.inputs()[0].displayedUs);
    p.engineApplied(1, 120000); p.frame(121000, Playing);
    REQUIRE(p.inputs().size() == 2); CHECK(p.inputs()[0].registeredUs - p.inputs()[0].dequeuedUs == 10);
    CHECK(p.inputs()[0].appliedUs == 120000); CHECK(p.inputs()[0].displayedUs == 121000);
    CHECK_FALSE(p.inputs()[1].accepted); CHECK_FALSE(p.inputs()[1].appliedUs); CHECK_FALSE(p.inputs()[1].displayedUs);
}
TEST_CASE("Queued inputs remain separate and restarts cannot display old inputs", "[probe]") {
    PerformanceProbe p; p.beginSession(); p.input(1, 2, true, "Up"); p.input(3, 4, true, "Left");
    p.engineApplied(1, 100); p.frame(110, Playing);
    CHECK(p.inputs()[0].displayedUs == 110); CHECK_FALSE(p.inputs()[1].displayedUs);
    p.beginSession(); p.input(200, 201, true, "Down"); p.engineApplied(1, 300); p.frame(310, Playing);
    CHECK_FALSE(p.inputs()[1].displayedUs); CHECK(p.inputs()[2].displayedUs == 310);
    CHECK(p.inputs()[2].session == 2); CHECK(p.inputs()[2].ordinal == 1);
}
TEST_CASE("Invalid clocks and oversized runs are visible not silently accepted", "[probe]") {
    PerformanceProbe p; p.frame(-1, Playing); p.frame(20, Playing); p.frame(19, Playing);
    p.frame(PerformanceProbe::MaxTimestampUs + 1, Playing);
    p.input(30, 29, true, "Up"); p.input(-1, 100, true, "Up"); p.input(40, 50, false, "not-a-direction");
    CHECK(p.invalidSamples() == 6); CHECK(p.inputs().empty()); CHECK(p.windows().empty());
}
TEST_CASE("Probe storage is capped and reports drops", "[probe]") {
    PerformanceProbe p; p.frame(0, Playing); p.frame(7200LL * 1000000, Playing);
    CHECK(p.windows().size() == PerformanceProbe::MaxWindows); CHECK(p.droppedWindows() == 3600);
    for (std::size_t i = 0; i < PerformanceProbe::MaxInputs + 10; ++i)
        p.input(static_cast<std::int64_t>(i) * 2, static_cast<std::int64_t>(i) * 2 + 1, false, "Up");
    CHECK(p.inputs().size() == PerformanceProbe::MaxInputs); CHECK(p.droppedInputs() == 10);
}
TEST_CASE("Engine and controller acknowledge only actually queued directions", "[probe][gameplay]") {
    GameEngine engine({{5, 5}, {4, 5}, {3, 5}}, Direction::Right, {20, 15});
    CHECK_FALSE(engine.tryQueueDirection(Direction::Right)); CHECK_FALSE(engine.tryQueueDirection(Direction::Left));
    CHECK(engine.tryQueueDirection(Direction::Up)); CHECK(engine.tryQueueDirection(Direction::Left));
    CHECK_FALSE(engine.tryQueueDirection(Direction::Down)); CHECK(engine.appliedDirectionCount() == 0);
    engine.tick(); CHECK(engine.appliedDirectionCount() == 1); CHECK(engine.direction() == Direction::Up);
    engine.tick(); CHECK(engine.appliedDirectionCount() == 2); CHECK(engine.direction() == Direction::Left);
    engine.endGame(GameOverReason::TIME_UP); CHECK_FALSE(engine.tryQueueDirection(Direction::Down));
    GameplayController game; CHECK_FALSE(game.requestDirection(Direction::Up));
    REQUIRE(game.start({})); CHECK(game.requestDirection(Direction::Up));
    CHECK_FALSE(game.requestDirection(Direction::Down));
}
TEST_CASE("Report has explicit scope, null unobserved inputs, and never overwrites files", "[probe]") {
    PerformanceProbe p; p.beginSession(); p.input(10, 20, true, "Up"); p.frame(50, Playing);
    ReportFile file; std::string error; REQUIRE(p.writeReport(file.path, error));
    std::ifstream input(file.path); const auto report = nlohmann::json::parse(input);
    CHECK(report["acceptance"] == "not_assessed"); CHECK(report["first_frame_us"] == 50);
    CHECK(report["powerups_implemented"] == false); CHECK(report["fps_windows"].empty());
    CHECK(report["inputs"][0]["displayed_us"].is_null());
    CHECK_FALSE(p.writeReport(file.path, error)); CHECK_FALSE(error.empty());
    CHECK_FALSE(p.writeReport(file.path / "missing" / "out.json", error));
}
