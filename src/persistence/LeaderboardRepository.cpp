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
#include <string_view>
#include <utility>
#include <vector>

namespace snake {
namespace {

std::string currentUtcDate() {
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
    output << std::put_time(&utcTime, "%Y-%m-%d");
    return output.str();
}

bool isValidCalendarDate(std::string_view date) noexcept {
    if (date.size() != 10U || date[4] != '-' || date[7] != '-') {
        return false;
    }

    for (std::size_t index = 0; index < date.size(); ++index) {
        if (index == 4U || index == 7U) {
            continue;
        }

        if (date[index] < '0' || date[index] > '9') {
            return false;
        }
    }

    const int year =
        (date[0] - '0') * 1000 +
        (date[1] - '0') * 100 +
        (date[2] - '0') * 10 +
        (date[3] - '0');
    const int month = (date[5] - '0') * 10 + (date[6] - '0');
    const int day = (date[8] - '0') * 10 + (date[9] - '0');

    // The schema uses the proleptic Gregorian calendar, years 0001-9999.
    if (year == 0 || month < 1 || month > 12 || day < 1) {
        return false;
    }

    static constexpr int daysInMonth[] = {
        31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
    };

    int maximumDay = daysInMonth[month - 1];
    const bool leapYear =
        (year % 4 == 0) && ((year % 100 != 0) || (year % 400 == 0));

    if (month == 2 && leapYear) {
        maximumDay = 29;
    }

    return day <= maximumDay;
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
LeaderboardRepository::loadValidatedEntries(bool* corruptJson) const {
    if (corruptJson != nullptr) {
        *corruptJson = false;
    }

    const StoreReadResult result = store_.readJson();

    // Missing or empty files represent a new leaderboard.
    if (result.status == StoreReadStatus::NotFound ||
        result.status == StoreReadStatus::EmptyFile) {
        return std::vector<LeaderboardEntry>{};
    }

    // Syntactically malformed JSON can be recovered by addScore().
    // Unsafe JSON, unsafe paths, oversized files and I/O failures are
    // deliberately kept distinct and are never overwritten as recovery.
    if (result.status == StoreReadStatus::InvalidJson) {
        if (corruptJson != nullptr) {
            *corruptJson = true;
            return std::vector<LeaderboardEntry>{};
        }

        return std::nullopt;
    }

    if (result.status != StoreReadStatus::Ok) {
        return std::nullopt;
    }

    const auto invalidSchema =
        [corruptJson]() -> std::optional<std::vector<LeaderboardEntry>> {
            if (corruptJson != nullptr) {
                *corruptJson = true;
                return std::vector<LeaderboardEntry>{};
            }

            return std::nullopt;
        };

    const auto& document = result.value;

    if (!document.is_object() ||
        !document.contains("version") ||
        !isValidVersion(document.at("version")) ||
        !document.contains("entries") ||
        !document.at("entries").is_array()) {
        return invalidSchema();
    }

    const auto& serializedEntries = document.at("entries");

    // An on-disk schema may contain at most ten entries. Do not silently
    // accept a larger document and display a truncated Top 10.
    if (serializedEntries.size() > 10U) {
        return invalidSchema();
    }

    std::vector<LeaderboardEntry> entries;
    entries.reserve(serializedEntries.size());

    for (const auto& item : serializedEntries) {
        if (!item.is_object() ||
            !item.contains("player_name") ||
            !item.contains("score") ||
            !item.contains("game_mode") ||
            !item.contains("difficulty") ||
            !item.contains("date_achieved")) {
            return invalidSchema();
        }

        if (!item.at("player_name").is_string() ||
            !item.at("game_mode").is_string() ||
            !item.at("difficulty").is_string() ||
            !item.at("date_achieved").is_string()) {
            return invalidSchema();
        }

        const auto& scoreValue = item.at("score");
        if (!scoreValue.is_number_integer() &&
            !scoreValue.is_number_unsigned()) {
            return invalidSchema();
        }

        std::int64_t score64 = 0;
        try {
            score64 = scoreValue.get<std::int64_t>();
        } catch (const nlohmann::json::exception&) {
            return invalidSchema();
        }

        if (score64 < 0 ||
            score64 > std::numeric_limits<int>::max()) {
            return invalidSchema();
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

        if (!isValidPlayerName(name) ||
            sanitizePlayerName(name) != name ||
            !mode.has_value() ||
            !difficulty.has_value() ||
            !isValidCalendarDate(date)) {
            return invalidSchema();
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
    if (warning != nullptr) {
        warning->clear();
    }

    auto entries = loadValidatedEntries();

    if (!entries) {
        if (warning != nullptr) {
            *warning =
                "E_LEADERBOARD_READ: invalid or unreadable board; empty table shown";
        }
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
            std::nullopt,
            {}
        };
    };

    if (!isValidPlayerName(playerName) || score < 0) {
        return errorResult();
    }

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

    std::string normalizedName = sanitizePlayerName(playerName);
    if (normalizedName.empty() ||
        !isValidPlayerName(normalizedName)) {
        return errorResult();
    }

    bool recoveredCorruptJson = false;
    auto entries = loadValidatedEntries(&recoveredCorruptJson);

    // Only malformed JSON and schema-invalid JSON opt into recovery.
    // Unsafe paths, UnsafeJson, TooLarge, and I/O failures return Error
    // without replacing the existing file.
    if (!entries) {
        return errorResult();
    }

    auto ranked = rankedTop10(std::move(*entries));

    // A full leaderboard requires a score strictly above tenth place.
    if (ranked.size() == 10U &&
        score <= ranked.back().score) {
        return ScoreSubmissionResult{
            ScoreSubmissionStatus::NotQualified,
            std::nullopt,
            {}
        };
    }

    const std::size_t rank = 1U + static_cast<std::size_t>(
        std::count_if(
            ranked.begin(),
            ranked.end(),
            [score](const auto& entry) {
                return entry.score >= score;
            }));

    const std::string achievedAt = currentUtcDate();
    if (!isValidCalendarDate(achievedAt)) {
        return errorResult();
    }

    ranked.push_back(LeaderboardEntry{
        std::move(normalizedName),
        score,
        gameMode,
        difficulty,
        achievedAt
    });
    ranked = rankedTop10(std::move(ranked));

    if (!store_.writeJson(serializeEntries(ranked))) {
        return errorResult();
    }

    return ScoreSubmissionResult{
        ScoreSubmissionStatus::Submitted,
        rank,
        recoveredCorruptJson
            ? "Warning: the previous leaderboard JSON was malformed or "
              "schema-invalid. A fresh leaderboard was saved."
            : std::string{}
    };
}

} // namespace snake
