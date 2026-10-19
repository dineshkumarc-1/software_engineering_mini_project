
#pragma once

namespace snake {

enum class GameMode {
    Classic,
    Timed,
    Obstacle
};

class ScoreService {
public:
    void reset() noexcept;
    void foodEaten(GameMode mode) noexcept;

    [[nodiscard]] int currentScore() const noexcept;
    [[nodiscard]] int highScore() const noexcept;

private:
    int currentScore_{0};
    int highScore_{0};
};

} // namespace snake
