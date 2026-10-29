#pragma once

namespace snake {
// Navigation only: no SFML types or game rules.
enum class Screen { MainMenu, NameEntry, ModeSelection, Gameplay, Paused,
                    GameOver, Leaderboard, Settings, Exiting };
enum class Action { Start, ConfirmName, BeginGame, Pause, Resume, EndGame,
                    Restart, OpenLeaderboard, OpenSettings, Back, BackToMenu, Quit };
class AppState {
public:
    [[nodiscard]] Screen screen() const noexcept { return screen_; }
    void dispatch(Action action) noexcept;
private:
    Screen screen_{Screen::MainMenu};
};
} // namespace snake
