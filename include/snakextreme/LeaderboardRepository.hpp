#pragma once

#include "snakextreme/GameMode.hpp"
#include "snakextreme/SecureFileStore.hpp"

#include <cstddef>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace snake {

// Version of the persisted leaderboard JSON schema.
inline constexpr int LeaderboardSchemaVersion = 1;

struct LeaderboardEntry {
    std::string playerName;
    int score{0};
    GameMode gameMode{DefaultGameMode};
    Difficulty difficulty{DefaultDifficulty};
    std::string dateAchieved;
};
enum class ScoreSubmissionStatus {
    Submitted,
    NotQualified,
    Error
};

struct ScoreSubmissionResult {
    ScoreSubmissionStatus status{
        ScoreSubmissionStatus::Error
    };

    std::optional<std::size_t> rank{};

    std::string warning{};
};

class LeaderboardRepository {
public:
    explicit LeaderboardRepository(
        std::filesystem::path filePath);

    // Returns up to 10 entries, sorted by descending score. If data cannot
    // be loaded safely, returns an empty table and optionally reports a warning.
    [[nodiscard]] std::vector<LeaderboardEntry>
    top10(std::string* warning = nullptr) const;

    // Validates and normalizes the name before saving.
    // Game mode and difficulty describe the completed session.
        [[nodiscard]] ScoreSubmissionResult addScore(
        std::string_view playerName,
        int score,
        GameMode gameMode,
        Difficulty difficulty);

private:
    [[nodiscard]] std::optional<std::vector<LeaderboardEntry>>
    loadValidatedEntries(
        bool* corruptJson = nullptr,
        bool* emptyFile = nullptr) const;
    [[nodiscard]]
    static std::vector<LeaderboardEntry> rankedTop10(
        std::vector<LeaderboardEntry> entries);

    SecureFileStore store_;
};

} // namespace snake
