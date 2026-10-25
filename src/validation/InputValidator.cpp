#include "snakextreme/InputValidator.hpp"

namespace snake {

namespace {

bool isAllowedPlayerNameCharacter(char ch) noexcept {
    return (ch >= 'A' && ch <= 'Z') ||
           (ch >= 'a' && ch <= 'z') ||
           (ch >= '0' && ch <= '9') ||
           ch == '_' ||
           ch == '-' ||
           ch == ' ';
}

} // namespace

bool isValidPlayerName(std::string_view name) noexcept {
    if (name.empty() || name.size() > MaxPlayerNameLength) {
        return false;
    }

    for (const char ch : name) {
        if (!isAllowedPlayerNameCharacter(ch)) {
            return false;
        }
    }

    return true;
}

std::string sanitizePlayerName(std::string_view name) {
    std::string sanitized;
    sanitized.reserve(
        name.size() < MaxPlayerNameLength
            ? name.size()
            : MaxPlayerNameLength);

    for (const char ch : name) {
        if (sanitized.size() == MaxPlayerNameLength) {
            break;
        }

        if (isAllowedPlayerNameCharacter(ch)) {
            sanitized.push_back(ch);
        }
    }

    return sanitized;
}

} // namespace snake