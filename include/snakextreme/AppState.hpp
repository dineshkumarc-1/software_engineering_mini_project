#pragma once

namespace snake {
// Application navigation only. No game rules, window objects, or SFML types.
enum class Screen { MainMenu, Gameplay, Paused, Exiting };
enum class Action { Start, Pause, Resume, BackToMenu, Quit };

class AppState {
public:
    [[nodiscard]] Screen screen() const noexcept { return screen_; }
    void dispatch(Action action) noexcept;
private:
    Screen screen_{Screen::MainMenu};
};
} // namespace snake
