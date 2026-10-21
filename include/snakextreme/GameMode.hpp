#pragma once

#include <array>
#include <optional>
#include <string_view>

namespace snake {

// FR-008: the three selectable game modes. The string forms below are the
// enum values stored in leaderboard.json (SRS Appendix B).
enum class GameMode {
    Classic,
    Timed,
    Obstacle
};

// FR-009: the three difficulty levels. Speeds live in DifficultyProfile.hpp.
enum class Difficulty {
    Easy,
    Medium,
    Hard
};

// Menu order. Exactly three modes and three difficulties are offered.
inline constexpr std::array<GameMode, 3> AllGameModes{
    GameMode::Classic, GameMode::Timed, GameMode::Obstacle};
inline constexpr std::array<Difficulty, 3> AllDifficulties{
    Difficulty::Easy, Difficulty::Medium, Difficulty::Hard};

// Pre-selected options on the Mode & Difficulty screen (TC-MODE-01).
inline constexpr GameMode DefaultGameMode = GameMode::Classic;
inline constexpr Difficulty DefaultDifficulty = Difficulty::Medium;

constexpr std::string_view toString(GameMode mode) noexcept {
    switch (mode) {
    case GameMode::Classic:
        return "Classic";
    case GameMode::Timed:
        return "Timed";
    case GameMode::Obstacle:
        return "Obstacle";
    }
    return "Classic";
}

constexpr std::string_view toString(Difficulty difficulty) noexcept {
    switch (difficulty) {
    case Difficulty::Easy:
        return "Easy";
    case Difficulty::Medium:
        return "Medium";
    case Difficulty::Hard:
        return "Hard";
    }
    return "Medium";
}

// Exact, case-sensitive match of the stored enum strings; anything else is
// rejected so persistence code can treat it as a schema error (SEC-003).
constexpr std::optional<GameMode> parseGameMode(std::string_view text) noexcept {
    for (const GameMode mode : AllGameModes) {
        if (toString(mode) == text) {
            return mode;
        }
    }
    return std::nullopt;
}

constexpr std::optional<Difficulty> parseDifficulty(std::string_view text) noexcept {
    for (const Difficulty difficulty : AllDifficulties) {
        if (toString(difficulty) == text) {
            return difficulty;
        }
    }
    return std::nullopt;
}

} // namespace snake
