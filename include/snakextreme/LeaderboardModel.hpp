#pragma once
#include "snakextreme/LeaderboardRepository.hpp"
#include <utility>

namespace snake {
// SFML-free application adapter: one submission per completed session.
// Refreshing the table or re-rendering Game Over never resubmits a score.
class LeaderboardModel {
public:
    explicit LeaderboardModel(std::filesystem::path path = "leaderboard.json")
        : repository_(std::move(path)) {}
    void beginSession() { submitted_ = false; message_.clear(); }
    void finish(std::string_view name, int score, GameMode mode, Difficulty difficulty);
    void refresh();
    const std::vector<LeaderboardEntry>& entries() const noexcept { return entries_; }
    const std::string& message() const noexcept { return message_; }
private:
    LeaderboardRepository repository_;
    bool submitted_{false};
    std::vector<LeaderboardEntry> entries_;
    std::string message_;
};
} // namespace snake
