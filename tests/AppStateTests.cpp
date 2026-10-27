#include "snakextreme/AppState.hpp"
#include "snakextreme/NameEntry.hpp"
#include <cstdlib>
#include <iostream>

namespace {
int checks = 0;
void expect(bool condition, const char* message) {
    ++checks;
    if (!condition) { std::cerr << "FAIL: " << message << '\n'; std::exit(1); }
}
void enterGame(snake::AppState& state) {
    state.dispatch(snake::Action::Start);
    state.dispatch(snake::Action::ConfirmName);
    state.dispatch(snake::Action::BeginGame);
}
}
int main() {
    using namespace snake;
    AppState state;
    expect(state.screen() == Screen::MainMenu, "launch menu");
    state.dispatch(Action::BeginGame);
    expect(state.screen() == Screen::MainMenu, "cannot bypass name and selection");
    state.dispatch(Action::Start);
    expect(state.screen() == Screen::NameEntry, "new game opens name entry");
    state.dispatch(Action::BeginGame);
    expect(state.screen() == Screen::NameEntry, "name entry cannot directly launch");
    state.dispatch(Action::ConfirmName);
    expect(state.screen() == Screen::ModeSelection, "confirmed name opens selection");
    state.dispatch(Action::Back);
    expect(state.screen() == Screen::NameEntry, "selection back edits name");
    state.dispatch(Action::Back);
    expect(state.screen() == Screen::MainMenu, "name cancel returns menu");
    enterGame(state);
    expect(state.screen() == Screen::Gameplay, "three-screen launch path");
    state.dispatch(Action::Start);
    expect(state.screen() == Screen::Gameplay, "new game ignored while playing");
    state.dispatch(Action::Pause);
    expect(state.screen() == Screen::Paused, "pause");
    state.dispatch(Action::Resume);
    expect(state.screen() == Screen::Gameplay, "resume");
    state.dispatch(Action::EndGame);
    expect(state.screen() == Screen::GameOver, "explicit game-over screen");
    state.dispatch(Action::Pause);
    expect(state.screen() == Screen::GameOver, "finished game cannot pause");
    state.dispatch(Action::Restart);
    expect(state.screen() == Screen::Gameplay, "restart");
    state.dispatch(Action::BackToMenu);
    for (const Action open : {Action::OpenLeaderboard, Action::OpenSettings}) {
        state.dispatch(open);
        expect(state.screen() == (open == Action::OpenLeaderboard ? Screen::Leaderboard : Screen::Settings), "menu destination");
        state.dispatch(Action::Back);
        expect(state.screen() == Screen::MainMenu, "back navigation");
    }
    // Check Quit from every reachable screen, and terminal-state behavior.
    for (int route = 0; route < 8; ++route) {
        AppState app;
        if (route == 1) app.dispatch(Action::Start);
        if (route == 2) { app.dispatch(Action::Start); app.dispatch(Action::ConfirmName); }
        if (route >= 3 && route <= 5) enterGame(app);
        if (route == 4) app.dispatch(Action::Pause);
        if (route == 5) app.dispatch(Action::EndGame);
        if (route == 6) app.dispatch(Action::OpenLeaderboard);
        if (route == 7) app.dispatch(Action::OpenSettings);
        app.dispatch(Action::Quit);
        expect(app.screen() == Screen::Exiting, "quit from all screens");
        app.dispatch(Action::BackToMenu);
        app.dispatch(Action::Start);
        expect(app.screen() == Screen::Exiting, "exit is terminal");
    }
    NameEntry entry;
    expect(!entry.confirm(), "empty name rejected");
    entry.input(' '); entry.input(' ');
    expect(!entry.confirm(), "blank name rejected");
    entry.input(8); entry.input(8);
    for (const char ch : std::string(" Dinesh_1- ")) entry.input(static_cast<unsigned char>(ch));
    expect(entry.confirm() && entry.value() == "Dinesh_1-", "trimmed valid name");
    const auto before = entry.value();
    for (auto cp : {0u, 9u, 47u, 64u, 0x1f600u, 0xf1u}) {
        entry.input(cp);
        expect(entry.value() == before && !entry.error().empty(), "invalid codepoint rejected with error");
    }
    entry = NameEntry{};
    for (int i = 0; i < 10000; ++i) entry.input('A');
    expect(entry.value().size() == 15, "10000 inputs bounded at 15");
    expect(entry.confirm(), "15 character name accepted");
    entry.input(8);
    expect(entry.value().size() == 14, "backspace");
    std::cout << checks << " navigation/name-entry checks passed\n";
}
