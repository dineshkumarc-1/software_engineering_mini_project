#pragma once
// Pixel-space controls shared by rendering and input; no SFML dependency.
#include "snakextreme/AppState.hpp"
#include <cstddef>
#include <optional>
#include <vector>
namespace snake {
struct UiRect {
    int left, top, width, height;
    bool contains(int x, int y) const noexcept {
        return x >= left && x < left + width && y >= top && y < top + height;
    }
};
enum class UiCommand { MainItem, Mode, Difficulty, Confirm, Back, Resume, Restart, Menu };
struct UiButton { UiRect area; UiCommand command; std::size_t index; const char* label; };
inline std::vector<UiButton> footerButtons(Screen screen) {
    const UiRect left{260, 435, 180, 42}, right{460, 435, 180, 42};
    switch (screen) {
    case Screen::NameEntry: return {{left, UiCommand::Back, 0, "Back"}, {right, UiCommand::Confirm, 0, "Continue"}};
    case Screen::ModeSelection: return {{left, UiCommand::Back, 0, "Back"}, {right, UiCommand::Confirm, 0, "Start game"}};
    case Screen::Leaderboard: return {{left, UiCommand::Back, 0, "Back"}};
    case Screen::Paused: return {{{260, 548, 180, 36}, UiCommand::Menu, 0, "Main menu"}, {{460, 548, 180, 36}, UiCommand::Resume, 0, "Resume"}};
    case Screen::GameOver: return {{{260, 548, 180, 36}, UiCommand::Menu, 0, "Main menu"}, {{460, 548, 180, 36}, UiCommand::Restart, 0, "Play again"}};
    default: return {};
    }
}
inline std::optional<UiButton> navigationHit(Screen screen, int x, int y) {
    if (screen == Screen::MainMenu) {
        const char* labels[] = {"New Game", "Leaderboard", "Settings", "Exit"};
        for (std::size_t i = 0; i < 4; ++i) {
            UiButton button{{260, 190 + static_cast<int>(i) * 60, 380, 48}, UiCommand::MainItem, i, labels[i]};
            if (button.area.contains(x, y)) return button;
        }
    }
    if (screen == Screen::ModeSelection) {
        for (std::size_t i = 0; i < 3; ++i) {
            if (UiRect{160 + static_cast<int>(i) * 200, 200, 180, 48}.contains(x, y))
                return UiButton{{}, UiCommand::Mode, i, ""};
            if (UiRect{160 + static_cast<int>(i) * 200, 338, 180, 48}.contains(x, y))
                return UiButton{{}, UiCommand::Difficulty, i, ""};
        }
    }
    for (const auto& button : footerButtons(screen)) if (button.area.contains(x, y)) return button;
    return std::nullopt;
}
inline std::optional<std::size_t> settingsRowAt(int x, int y) {
    if (x < 160 || x >= 740 || y < 120 || y >= 470 || (y - 120) % 35 >= 32) return std::nullopt;
    return static_cast<std::size_t>((y - 120) / 35);
}
} // namespace snake
