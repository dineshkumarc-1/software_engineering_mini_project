
#pragma once

#include <cstddef>
#include <string>
#include <string_view>

namespace snake {

inline constexpr std::size_t MaxPlayerNameLength = 15;

// Checks whether one character is allowed in a player name.
// A space is allowed here, even though a name containing only
// spaces is invalid.
[[nodiscard]] bool isAllowedPlayerNameCharacter(char ch) noexcept;

// Validates a name after considering leading/trailing spaces.
// The effective name must be non-empty and at most 15 characters.
[[nodiscard]] bool isValidPlayerName(std::string_view name) noexcept;

// Removes disallowed characters, trims outer spaces, and
// limits the resulting name to 15 characters.
[[nodiscard]] std::string sanitizePlayerName(std::string_view name);

} // namespace snake
