
#pragma once

#include "snakextreme/GameMode.hpp"

namespace snake {

class ScoreService {
public:
    // Starts a new game: score 0 and no multiplier (the high score stays).
    void reset() noexcept;
    // Adds the mode's points for one food, times the current multiplier.
    void foodEaten(GameMode mode) noexcept;

    // FR-014: multiplier for the points of every following food, set by the
    // Multiplier power-up (2) and reset when it ends (1). Values below 1
    // count as 1.
    void setMultiplier(int multiplier) noexcept;
    [[nodiscard]] int multiplier() const noexcept;

    [[nodiscard]] int currentScore() const noexcept;
    [[nodiscard]] int highScore() const noexcept;

private:
    int currentScore_{0};
    int highScore_{0};
    int multiplier_{1};
};

} // namespace snake
