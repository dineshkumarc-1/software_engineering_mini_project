
#include "snakextreme/InputValidator.hpp"

namespace snake {

bool isAllowedPlayerNameCharacter(char ch) noexcept {
    return (ch >= 'A' && ch <= 'Z') ||
           (ch >= 'a' && ch <= 'z') ||
           (ch >= '0' && ch <= '9') ||
           ch == '_' ||
           ch == '-' ||
           ch == ' ';
}

bool isValidPlayerName(std::string_view name) noexcept {
    if (name.empty()) {
        return false;
    }

    // Reject characters outside the whitelist.
    for (const char ch : name) {
        if (!isAllowedPlayerNameCharacter(ch)) {
            return false;
        }
    }

    // Ignore outer spaces when checking the effective name.
    const std::size_t first = name.find_first_not_of(' ');
    if (first == std::string_view::npos) {
        return false;
    }

    const std::size_t last = name.find_last_not_of(' ');
    const std::size_t trimmedLength = last - first + 1;

    return trimmedLength <= MaxPlayerNameLength;
}

std::string sanitizePlayerName(std::string_view name) {
    std::string sanitized;
    sanitized.reserve(
        name.size() < MaxPlayerNameLength
            ? name.size()
            : MaxPlayerNameLength);

    // Keep only permitted characters.
    for (const char ch : name) {
        if (isAllowedPlayerNameCharacter(ch)) {
            sanitized.push_back(ch);
        }
    }

    // Trim leading spaces.
    const std::size_t first = sanitized.find_first_not_of(' ');
    if (first == std::string::npos) {
        return {};
    }
    sanitized.erase(0, first);

    // Trim trailing spaces.
    const std::size_t last = sanitized.find_last_not_of(' ');
    sanitized.resize(last + 1);

    // Enforce the maximum length and remove any trailing space
    // introduced by truncation.
    if (sanitized.size() > MaxPlayerNameLength) {
        sanitized.resize(MaxPlayerNameLength);

        while (!sanitized.empty() && sanitized.back() == ' ') {
            sanitized.pop_back();
        }
    }

    return sanitized;
}

} // namespace snake
