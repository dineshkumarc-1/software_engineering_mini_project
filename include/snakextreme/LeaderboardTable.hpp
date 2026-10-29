#pragma once

#include "snakextreme/LeaderboardRepository.hpp"
#include <array>
#include <cstddef>
#include <string>
#include <string_view>

namespace snake {
// TC-LB-02: keep the rendered table and the text fallback on the same six fields.
// Coordinates are in the existing 900 x 600 logical navigation view.
struct LeaderboardColumn {
    std::string_view heading;
    float left;
    float width;
};
inline constexpr std::array<LeaderboardColumn, 6> LeaderboardColumns{{
    {"Rank", 70.f, 40.f}, {"Player name", 120.f, 210.f},
    {"Score", 340.f, 105.f}, {"Mode", 455.f, 100.f},
    {"Difficulty", 565.f, 95.f}, {"Date", 670.f, 145.f}
}};
inline std::array<std::string, 6> leaderboardRow(std::size_t rank,
                                               const LeaderboardEntry& entry) {
    return {std::to_string(rank), entry.playerName, std::to_string(entry.score),
            std::string(toString(entry.gameMode)),
            std::string(toString(entry.difficulty)), entry.dateAchieved};
}
// Keep the old rank/name/score segment intact for existing QA title parsers.
// Metadata follows a separate delimiter, so all six fields remain available
// even when the bundled font cannot be loaded.
inline std::string leaderboardRowTitle(std::size_t rank, const LeaderboardEntry& entry) {
    const auto row = leaderboardRow(rank, entry);
    return " | " + row[0] + ". " + row[1] + " " + row[2] +
        " | " + row[3] + " / " + row[4] + " / " + row[5];
}
} // namespace snake
