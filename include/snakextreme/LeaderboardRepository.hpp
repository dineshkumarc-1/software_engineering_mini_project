#pragma once

#include "snakextreme/SecureFileStore.hpp"

#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace snake {

struct LeaderboardEntry {
    std::string playerName;
    int score{0};
};

class LeaderboardRepository {
public:
    explicit LeaderboardRepository(
        std::filesystem::path filePath);

    // Returns validated entries sorted by descending score.
    // The result contains at most 10 entries.
    [[nodiscard]] std::vector<LeaderboardEntry>
    top10() const;

    // Rejects invalid names and negative scores.
    // Accepted names are normalized before saving.
    [[nodiscard]] bool addScore(
        std::string_view playerName,
        int score);

private:
    [[nodiscard]]
    std::optional<std::vector<LeaderboardEntry>>
    loadValidatedEntries() const;

    [[nodiscard]]
    static std::vector<LeaderboardEntry> rankedTop10(
        std::vector<LeaderboardEntry> entries);

    SecureFileStore store_;
};

} // namespace snake
