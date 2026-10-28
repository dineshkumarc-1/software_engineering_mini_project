#pragma once

#include "snakextreme/InputValidator.hpp"
#include <algorithm>
#include <array>
#include <cstdint>
#include <string>
#include <string_view>

namespace snake {
// SFML-free, physically fixed-capacity name input (SEC-006). C10 remains the
// single source of truth for allowed characters and complete-name validation.
class NameEntry {
public:
    static constexpr std::size_t Capacity = MaxPlayerNameLength;
    void input(std::uint32_t codepoint) {
        if (codepoint == 8) {
            if (length_ != 0) buffer_[--length_] = '\0';
            error_.clear();
            return;
        }
        if (codepoint == 13 || codepoint == 10) return;
        if (codepoint > 127 || !isAllowedPlayerNameCharacter(static_cast<char>(codepoint))) {
            error_ = "Use letters, numbers, spaces, underscore or hyphen.";
            return;
        }
        if (length_ == Capacity) {
            error_ = "Maximum 15 characters.";
            return;
        }
        buffer_[length_++] = static_cast<char>(codepoint);
        error_.clear();
    }
    bool confirm() {
        // Spaces are valid while typing; the shared complete-name validator
        // rejects empty/blank submissions before shared normalization.
        const std::string_view entered(buffer_.data(), length_);
        if (!isValidPlayerName(entered)) {
            error_ = "Enter a valid name (1-15 characters), not only spaces.";
            return false;
        }
        const auto normalized = sanitizePlayerName(entered);
        if (normalized.size() > Capacity || !isValidPlayerName(normalized)) {
            error_ = "Enter a valid name (1-15 characters), not only spaces.";
            return false;
        }
        buffer_.fill('\0');
        std::copy(normalized.begin(), normalized.end(), buffer_.begin());
        length_ = normalized.size();
        error_.clear();
        return true;
    }
    // Return a display snapshot, not a mutable view or a pointer requiring a
    // terminator. The input's backing storage itself never grows or allocates.
    std::string value() const { return {buffer_.data(), length_}; }
    const std::string& error() const noexcept { return error_; }
private:
    std::array<char, Capacity> buffer_{};
    std::size_t length_{0};
    std::string error_;
};
} // namespace snake
