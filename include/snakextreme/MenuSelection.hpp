#pragma once

// Main-menu choice of game mode and difficulty (FR-008, FR-009).
// SFML-free: AppManager maps keys to MenuInput, everything else is tested here.

#include "snakextreme/AppState.hpp"
#include "snakextreme/GameMode.hpp"
#include "snakextreme/ModeManager.hpp" // SessionConfig

#include <algorithm>
#include <cstddef>

namespace snake {

enum class MenuInput {
    PreviousMode,     // Left / A
    NextMode,         // Right / D
    HarderDifficulty, // Up / W
    EasierDifficulty  // Down / S
};

// Modes form a loop, so Left/Right wrap around.
constexpr GameMode cycleMode(GameMode mode, int step) noexcept {
    const int count = static_cast<int>(AllGameModes.size());
    int index = 0;
    for (int i = 0; i < count; ++i) {
        if (AllGameModes[static_cast<std::size_t>(i)] == mode) {
            index = i;
        }
    }
    index = ((index + step) % count + count) % count;
    return AllGameModes[static_cast<std::size_t>(index)];
}

// Difficulty is a scale, so Up/Down stop at Easy and Hard.
constexpr Difficulty stepDifficulty(Difficulty difficulty, int step) noexcept {
    const int count = static_cast<int>(AllDifficulties.size());
    int index = 0;
    for (int i = 0; i < count; ++i) {
        if (AllDifficulties[static_cast<std::size_t>(i)] == difficulty) {
            index = i;
        }
    }
    index = std::clamp(index + step, 0, count - 1);
    return AllDifficulties[static_cast<std::size_t>(index)];
}

// Applies one menu key to the selection. The selection only changes on the
// main menu; during a game (running or paused) it is fixed.
constexpr SessionConfig applyMenuInput(SessionConfig selection,
                                       MenuInput input,
                                       Screen screen) noexcept {
    if (screen != Screen::MainMenu) {
        return selection;
    }

    switch (input) {
    case MenuInput::PreviousMode:
        selection.mode = cycleMode(selection.mode, -1);
        break;
    case MenuInput::NextMode:
        selection.mode = cycleMode(selection.mode, 1);
        break;
    case MenuInput::HarderDifficulty:
        selection.difficulty = stepDifficulty(selection.difficulty, 1);
        break;
    case MenuInput::EasierDifficulty:
        selection.difficulty = stepDifficulty(selection.difficulty, -1);
        break;
    }
    return selection;
}

} // namespace snake
