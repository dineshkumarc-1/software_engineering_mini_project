#include "snakextreme/AppState.hpp"
#include "snakextreme/NameEntry.hpp"
#include "snakextreme/UiHitTest.hpp"
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
    // Integration regression for C10 PR #42 revision 517c169. Calling the
    // complete-name validator for one typed space would wrongly reject it.
    entry = NameEntry{};
    entry.input(' ');
    expect(entry.value() == " " && entry.error().empty(), "space accepted while typing");
    entry.input(' ');
    expect(entry.value() == "  " && entry.error().empty(), "multiple typed spaces retained");
    expect(!entry.confirm() && entry.value() == "  " && !entry.error().empty(),
           "shared validator rejects blank submission without mutating input");
    entry.input('A');
    expect(entry.error().empty(), "editing clears submission error");
    entry.input(' ');
    expect(entry.confirm() && entry.value() == "A", "shared sanitizer trims submitted name");
    expect(entry.confirm() && entry.value() == "A", "confirmation is idempotent");
    entry = NameEntry{};
    const std::string padded = "  C Dinesh_1-  ";
    for (const unsigned char ch : padded) entry.input(ch);
    expect(entry.value() == padded, "padded and internal spaces preserved while editing");
    expect(entry.confirm() && entry.value() == sanitizePlayerName(padded),
           "complete-name submission reuses shared normalization");
    expect(isValidPlayerName(entry.value()), "confirmed value satisfies C10 contract");
    // Every ASCII keystroke uses C10's character API, with only editing and
    // submit control characters handled specially by the input widget.
    for (unsigned cp = 0; cp < 128; ++cp) {
        if (cp == 8 || cp == 10 || cp == 13) continue;
        NameEntry character;
        character.input(cp);
        const bool allowed = isAllowedPlayerNameCharacter(static_cast<char>(cp));
        expect(allowed ? character.value() == std::string(1, static_cast<char>(cp)) && character.error().empty()
                       : character.value().empty() && !character.error().empty(),
               "ASCII entry agrees with shared per-character API");
    }
    entry = NameEntry{};
    for (int i = 0; i < 15; ++i) entry.input('B');
    const auto boundary = entry.value();
    entry.input('C');
    expect(entry.value() == boundary && !entry.error().empty(), "16th character rejected, not silently truncated");
    expect(entry.confirm() && entry.value() == boundary, "15-character submission remains intact");
    entry.input(13); entry.input(10);
    expect(entry.value() == boundary, "submit control events do not insert characters");
    for (int i = 0; i < 15; ++i) entry.input(8);
    expect(entry.value().empty() && !entry.confirm(), "backspacing to empty restores rejection");
    AppState submitted;
    submitted.dispatch(Action::Start);
    if (entry.confirm()) submitted.dispatch(Action::ConfirmName);
    expect(submitted.screen() == Screen::NameEntry, "invalid name cannot advance UI flow");
    for (const unsigned char ch : std::string(" Ayushi ")) entry.input(ch);
    if (entry.confirm()) submitted.dispatch(Action::ConfirmName);
    expect(submitted.screen() == Screen::ModeSelection && entry.value() == "Ayushi",
           "valid padded name advances UI flow with normalized value");
    for (Screen screen : {Screen::NameEntry, Screen::ModeSelection, Screen::Leaderboard, Screen::Paused, Screen::GameOver}) {
        for (const auto& button : footerButtons(screen)) {
            const auto& r = button.area;
            auto hit = navigationHit(screen, r.left, r.top);
            expect(hit && hit->command == button.command, "visible button top-left hits correct command");
            hit = navigationHit(screen, r.left + r.width - 1, r.top + r.height - 1);
            expect(hit && hit->command == button.command, "visible button last pixel hits correct command");
            expect(!navigationHit(screen, r.left + r.width, r.top), "button right edge excluded");
            expect(!navigationHit(screen, r.left, r.top + r.height), "button bottom edge excluded");
        }
    }
    for (std::size_t i = 0; i < 3; ++i) {
        auto hit = navigationHit(Screen::ModeSelection, 170 + static_cast<int>(i) * 200, 220);
        expect(hit && hit->command == UiCommand::Mode && hit->index == i, "mouse selects each mode");
        hit = navigationHit(Screen::ModeSelection, 170 + static_cast<int>(i) * 200, 350);
        expect(hit && hit->command == UiCommand::Difficulty && hit->index == i, "mouse selects each difficulty");
    }
    expect(!navigationHit(Screen::ModeSelection, 350, 220), "mode gap is inactive");
    expect(!navigationHit(Screen::Gameplay, 500, 450), "gameplay does not activate hidden menu controls");
    expect(!navigationHit(Screen::Settings, 500, 450), "settings has separate pointer handling");
    for (std::size_t i = 0; i < 10; ++i) {
        expect(settingsRowAt(160, 120 + static_cast<int>(i) * 35) == i, "settings row first pixel");
        expect(settingsRowAt(739, 151 + static_cast<int>(i) * 35) == i, "settings row last pixel");
        expect(!settingsRowAt(200, 152 + static_cast<int>(i) * 35), "settings row gap is inactive");
    }
    expect(!settingsRowAt(159, 120) && !settingsRowAt(740, 120) && !settingsRowAt(160, 470), "settings exterior excluded");
    std::cout << checks << " navigation/name-entry checks passed\n";
}
