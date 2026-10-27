#include "snakextreme/AppState.hpp"
namespace snake {
void AppState::dispatch(Action action) noexcept {
    if (screen_ == Screen::Exiting) return;
    if (action == Action::Quit) { screen_ = Screen::Exiting; return; }
    if (action == Action::BackToMenu) { screen_ = Screen::MainMenu; return; }
    switch (screen_) {
    case Screen::MainMenu:
        if (action == Action::Start) screen_ = Screen::NameEntry;
        if (action == Action::OpenLeaderboard) screen_ = Screen::Leaderboard;
        if (action == Action::OpenSettings) screen_ = Screen::Settings;
        break;
    case Screen::NameEntry:
        if (action == Action::ConfirmName) screen_ = Screen::ModeSelection;
        if (action == Action::Back) screen_ = Screen::MainMenu;
        break;
    case Screen::ModeSelection:
        if (action == Action::BeginGame) screen_ = Screen::Gameplay;
        if (action == Action::Back) screen_ = Screen::NameEntry;
        break;
    case Screen::Gameplay:
        if (action == Action::Pause) screen_ = Screen::Paused;
        if (action == Action::EndGame) screen_ = Screen::GameOver;
        break;
    case Screen::Paused:
        if (action == Action::Resume) screen_ = Screen::Gameplay;
        break;
    case Screen::GameOver:
        if (action == Action::Restart) screen_ = Screen::Gameplay;
        break;
    case Screen::Leaderboard:
    case Screen::Settings:
        if (action == Action::Back) screen_ = Screen::MainMenu;
        break;
    case Screen::Exiting: break;
    }
}
} // namespace snake
