#include "snakextreme/LeaderboardRepository.hpp"

#include "snakextreme/InputValidator.hpp"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <ctime>
#include <iomanip>
#include <limits>
#include <sstream>
#include <string>
#include <utility>

namespace snake {
namespace {

std::string currentUtcTimestamp() {
    const auto now = std::chrono::system_clock::now();
    const std::time_t timeValue =
        std::chrono::system_clock::to_time_t(now);

    std::tm utcTime{};

#ifdef _WIN32
    if (gmtime_s(&utcTime, &timeValue) != 0) {
        return {};
    }
#else
    if (gmtime_r(&timeValue, &utcTime) == nullptr) {
        return {};
    }
#endif

    std::ostringstream output;
    output << std::put_time(
        &utcTime, "%Y-%m-%dT%H:%M:%SZ");

    return output.str();
}

nlohmann::json serializeEntries(
    const std::vector<LeaderboardEntry>& entries) {

    nlohmann::json document;
    document["version"] = LeaderboardSchemaVersion;
    document["entries"] = nlohmann::json::array();

    for (const auto& entry : entries) {
        document["entries"].push_back({
            {"player_name", entry.playerName},
            {"score", entry.score},
            {"game_mode", std::string(toString(entry.gameMode))},
            {"difficulty", std::string(toString(entry.difficulty))},
            {"date_achieved", entry.dateAchieved}
        });
    }

    return document;
}

bool isValidVersion(const nlohmann::json& value) {
    if (!value.is_number_integer() &&
        !value.is_number_unsigned()) {
        return false;
    }

    try {
        return value.get<std::int64_t>() ==
            LeaderboardSchemaVersion;
    } catch (const nlohmann::json::exception&) {
        return false;
    }
}

} // namespace

LeaderboardRepository::LeaderboardRepository(
    std::filesystem::path filePath)
    : store_(std::move(filePath)) {}

std::optional<std::vector<LeaderboardEntry>>
LeaderboardRepository::loadValidatedEntries(
    bool* corruptJson) const {
      if (corruptJson != nullptr) {
    *corruptJson = false;
}
    const StoreReadResult result = store_.readJson();

    // A missing or empty file represents a new leaderboard.
    if (result.status == StoreReadStatus::NotFound ||
        result.status == StoreReadStatus::EmptyFile) {
        return std::vector<LeaderboardEntry>{};
    }

    // Do not silently overwrite malformed JSON.
    if (result.status == StoreReadStatus::InvalidJson) {
    // Only addScore() opts into recovery from malformed JSON.
    if (corruptJson != nullptr) {
        *corruptJson = true;
        return std::vector<LeaderboardEntry>{};
    }

    return std::nullopt;
}

    // Do not silently replace oversized files or ignore I/O errors.
    if (result.status != StoreReadStatus::Ok) {
        return std::nullopt;
    }

    const auto& document = result.value;

    // Validate the document and schema version.
    if (!document.is_object() ||
        !document.contains("version") ||
        !isValidVersion(document.at("version")) ||
        !document.contains("entries") ||
        !document.at("entries").is_array()) {
        return std::nullopt;
    }

    std::vector<LeaderboardEntry> entries;
    entries.reserve(document.at("entries").size());

    for (const auto& item : document.at("entries")) {
        if (!item.is_object() ||
            !item.contains("player_name") ||
            !item.contains("score") ||
            !item.contains("game_mode") ||
            !item.contains("difficulty") ||
            !item.contains("date_achieved")) {
            return std::nullopt;
        }

        if (!item.at("player_name").is_string() ||
            !item.at("game_mode").is_string() ||
            !item.at("difficulty").is_string() ||
            !item.at("date_achieved").is_string()) {
            return std::nullopt;
        }

        const auto& scoreValue = item.at("score");

        if (!scoreValue.is_number_integer() &&
            !scoreValue.is_number_unsigned()) {
            return std::nullopt;
        }

        std::int64_t score64 = 0;

        try {
            score64 = scoreValue.get<std::int64_t>();
        } catch (const nlohmann::json::exception&) {
            return std::nullopt;
        }

        if (score64 < 0 ||
            score64 > std::numeric_limits<int>::max()) {
            return std::nullopt;
        }

        const std::string name =
            item.at("player_name").get<std::string>();

        const std::string modeText =
            item.at("game_mode").get<std::string>();

        const std::string difficultyText =
            item.at("difficulty").get<std::string>();

        const std::string date =
            item.at("date_achieved").get<std::string>();

        const auto mode = parseGameMode(modeText);
        const auto difficulty = parseDifficulty(difficultyText);

        // Stored player names must already be valid and normalized.
        if (!isValidPlayerName(name) ||
            sanitizePlayerName(name) != name ||
            !mode.has_value() ||
            !difficulty.has_value() ||
            date.empty()) {
            return std::nullopt;
        }

        entries.push_back(LeaderboardEntry{
            name,
            static_cast<int>(score64),
            *mode,
            *difficulty,
            date
        });
    }

    return entries;
}

std::vector<LeaderboardEntry>
LeaderboardRepository::rankedTop10(
    std::vector<LeaderboardEntry> entries) {

    std::stable_sort(
        entries.begin(),
        entries.end(),
        [](const LeaderboardEntry& left,
           const LeaderboardEntry& right) {
            return left.score > right.score;
        });

    if (entries.size() > 10U) {
        entries.resize(10U);
    }

    return entries;
}

std::vector<LeaderboardEntry>
LeaderboardRepository::top10(std::string* warning) const {
    if (warning != nullptr) warning->clear();
    auto entries = loadValidatedEntries();

    if (!entries) {
        if (warning != nullptr) *warning = "E_LEADERBOARD_READ: invalid or unreadable board; empty table shown";
        return {};
    }

    return rankedTop10(std::move(*entries));
}


ScoreSubmissionResult LeaderboardRepository::addScore(
    std::string_view playerName,
    int score,
    GameMode gameMode,
    Difficulty difficulty) {

    const auto errorResult = [] {
        return ScoreSubmissionResult{
            ScoreSubmissionStatus::Error,
            std::nullopt
        };
    };

    // Validate the supplied name and score.
    if (!isValidPlayerName(playerName) || score < 0) {
        return errorResult();
    }

    // Reject invalid enum values.
    switch (gameMode) {
    case GameMode::Classic:
    case GameMode::Timed:
    case GameMode::Obstacle:
        break;
    default:
        return errorResult();
    }

    switch (difficulty) {
    case Difficulty::Easy:
    case Difficulty::Medium:
    case Difficulty::Hard:
        break;
    default:
        return errorResult();
    }

    std::string normalizedName =
        sanitizePlayerName(playerName);

    if (normalizedName.empty() ||
        !isValidPlayerName(normalizedName)) {
        return errorResult();
    }

    bool recoveredCorruptJson = false;

auto entries = loadValidatedEntries(&recoveredCorruptJson);

// Do not overwrite unreadable data or invalid schemas.
if (!entries) {
    return errorResult();
}

    // Rank the existing leaderboard before evaluating
    // whether the new score qualifies.
    auto ranked = rankedTop10(std::move(*entries));

    // On a full board, a score must be strictly greater
    // than tenth place. A tie at tenth place is rejected.
    if (ranked.size() == 10U &&
        score <= ranked.back().score) {
        return ScoreSubmissionResult{
            ScoreSubmissionStatus::NotQualified,
            std::nullopt
        };
    }

    // Equal scores rank after existing entries with that score.
    const std::size_t rank = 1U + static_cast<std::size_t>(std::count_if(
        ranked.begin(), ranked.end(), [score](const auto& entry) { return entry.score >= score; }));

    const std::string achievedAt = currentUtcTimestamp();

if (achievedAt.empty()) {
    return errorResult();
}

LeaderboardEntry newEntry{
    std::move(normalizedName),
    score,
    gameMode,
    difficulty,
    achievedAt
};

    ranked.push_back(std::move(newEntry));
    ranked = rankedTop10(std::move(ranked));

    // SecureFileStore writes the updated JSON atomically.
    if (!store_.writeJson(serializeEntries(ranked))) {
        return errorResult();
    }

    return ScoreSubmissionResult{
    ScoreSubmissionStatus::Submitted,
    rank,
    recoveredCorruptJson
        ? "Warning: the previous leaderboard JSON was malformed. "
          "A new leaderboard was saved."
        : std::string{}
};
}

} // namespace snake
