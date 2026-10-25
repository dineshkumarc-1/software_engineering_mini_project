#pragma once

#include <cstddef>
#include <string>
#include <string_view>

namespace snake {

inline constexpr std::size_t MaxPlayerNameLength = 15;

// Returns true only for non-empty names containing allowed characters
// and no more than MaxPlayerNameLength characters.
[[nodiscard]] bool isValidPlayerName(std::string_view name) noexcept;

// Removes disallowed characters and caps the result at 15 characters.
[[nodiscard]] std::string sanitizePlayerName(std::string_view name);

} // namespace snake