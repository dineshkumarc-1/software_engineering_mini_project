#include "snakextreme/LeaderboardModel.hpp"
#include <exception>

namespace snake {
void LeaderboardModel::finish(std::string_view name, int score, GameMode mode, Difficulty difficulty) {
    if (submitted_) return;
    submitted_ = true; // A failed save must not be retried every frame.
    try {
        const auto result = repository_.addScore(name, score, mode, difficulty);
        switch (result.status) {
        case ScoreSubmissionStatus::Submitted:
            message_ = "Score saved: rank " + std::to_string(result.rank.value_or(0));
            if (!result.warning.empty()) message_ += "; " + result.warning;
            break;
        case ScoreSubmissionStatus::NotQualified: message_ = "Score did not reach the Top 10"; break;
        case ScoreSubmissionStatus::Error: message_ = "E_LEADERBOARD_SAVE: score not saved"; break;
        }
    } catch (const std::exception&) {
        message_ = "E_LEADERBOARD_SAVE: score not saved";
    }
}
void LeaderboardModel::refresh() {
    try {
        entries_ = repository_.top10(&message_);
    } catch (const std::exception&) {
        entries_.clear();
        message_ = "E_LEADERBOARD_READ: empty table shown";
    }
}
} // namespace snake
