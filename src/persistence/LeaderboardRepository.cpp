#include "snakextreme/LeaderboardRepository.hpp"

#include "snakextreme/InputValidator.hpp"

#include <algorithm>
#include <cstdint>
#include <limits>
#include <utility>

namespace snake {
namespace {

nlohmann::json serializeEntries(
    const std::vector<LeaderboardEntry>& entries) {

    nlohmann::json document;
    document["entries"] = nlohmann::json::array();

    for (const auto& entry : entries) {
        document["entries"].push_back({
            {"playerName", entry.playerName},
            {"score", entry.score}
        });
    }

    return document;
}

} // namespace

LeaderboardRepository::LeaderboardRepository(
    std::filesystem::path filePath)
    : store_(std::move(filePath)) {}

std::optional<std::vector<LeaderboardEntry>>
LeaderboardRepository::loadValidatedEntries() const {

    const StoreReadResult result = store_.readJson();

    // A missing or empty file represents a new leaderboard.
if (result.status == StoreReadStatus::NotFound ||
    result.status == StoreReadStatus::EmptyFile) {
    return std::vector<LeaderboardEntry>{};
}

if (result.status == StoreReadStatus::InvalidJson) {
    // Reading can fall back to an empty display, but callers
    // must not silently overwrite the corrupt file.
    return std::nullopt;
}

    // Do not overwrite an oversized file or silently ignore
    // other I/O errors.
    if (result.status != StoreReadStatus::Ok) {
        return std::nullopt;
    }

    const auto& document = result.value;

    // Validate the top-level schema.
    if (!document.is_object() ||
        !document.contains("entries") ||
        !document.at("entries").is_array()) {
        return std::nullopt;
    }

    std::vector<LeaderboardEntry> entries;
    entries.reserve(document.at("entries").size());

    for (const auto& item : document.at("entries")) {
        if (!item.is_object() ||
            !item.contains("playerName") ||
            !item.contains("score") ||
            !item.at("playerName").is_string()) {
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
            item.at("playerName").get<std::string>();

        // Stored names must already be valid and normalized.
        // Do not silently repair a modified or crafted entry.
        if (!isValidPlayerName(name) ||
            sanitizePlayerName(name) != name) {
            return std::nullopt;
        }

        entries.push_back(LeaderboardEntry{
            name,
            static_cast<int>(score64)
        });
    }

    return entries;
}

std::vector<LeaderboardEntry>
LeaderboardRepository::rankedTop10(
    std::vector<LeaderboardEntry> entries) {

    // Highest score first. Stable sort preserves the relative
    // order of entries that have equal scores.
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
LeaderboardRepository::top10() const {

    auto entries = loadValidatedEntries();

    if (!entries) {
        return {};
    }

    return rankedTop10(std::move(*entries));
}

bool LeaderboardRepository::addScore(
    std::string_view playerName,
    int score) {

    // Validate the original input first.
    // Invalid symbols and whitespace-only names are rejected.
    if (!isValidPlayerName(playerName) || score < 0) {
        return false;
    }

    // Normalize accepted input, then validate it again before
    // it is serialized to persistent storage.
    std::string normalizedName =
        sanitizePlayerName(playerName);

    if (normalizedName.empty() ||
        !isValidPlayerName(normalizedName)) {
        return false;
    }

    auto entries = loadValidatedEntries();

    if (!entries) {
        return false;
    }

    entries->push_back(LeaderboardEntry{
        std::move(normalizedName),
        score
    });

    auto ranked = rankedTop10(std::move(*entries));

    // SecureFileStore enforces the 64 KiB limit and writes
    // through a temporary file followed by atomic replacement.
    return store_.writeJson(serializeEntries(ranked));
}

} // namespace snake
