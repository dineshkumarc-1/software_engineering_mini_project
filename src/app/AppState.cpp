#include "snakextreme/AppState.hpp"

namespace snake {
void AppState::dispatch(Action action) noexcept {
    if (screen_ == Screen::Exiting) return;
    if (action == Action::Quit) { screen_ = Screen::Exiting; return; }
    switch (screen_) {
    case Screen::MainMenu:
        if (action == Action::Start) screen_ = Screen::Gameplay;
        break;
    case Screen::Gameplay:
        if (action == Action::Pause) screen_ = Screen::Paused;
        if (action == Action::BackToMenu) screen_ = Screen::MainMenu;
        break;
    case Screen::Paused:
        if (action == Action::Resume) screen_ = Screen::Gameplay;
        if (action == Action::BackToMenu) screen_ = Screen::MainMenu;
        break;
    case Screen::Exiting: break;
    }
}
} // namespace snake
