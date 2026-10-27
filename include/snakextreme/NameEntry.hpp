#pragma once

#include "snakextreme/InputValidator.hpp"
#include <cstdint>
#include <string>

namespace snake {
// SFML-free text-entry model. Reuses C10 rather than duplicating its whitelist.
class NameEntry {
public:
    void input(std::uint32_t codepoint) {
        if (codepoint == 8) {
            if (!value_.empty()) value_.pop_back();
            error_.clear();
            return;
        }
        if (codepoint == 13 || codepoint == 10) return;
        if (codepoint > 127 || !isAllowedPlayerNameCharacter(static_cast<char>(codepoint))) {
            error_ = "Use letters, numbers, spaces, underscore or hyphen.";
            return;
        }
        if (value_.size() >= MaxPlayerNameLength) {
            error_ = "Maximum 15 characters.";
            return;
        }
        value_.push_back(static_cast<char>(codepoint));
        error_.clear();
    }
    bool confirm() {
        // Character entry and complete-name validation have different contracts:
        // spaces are valid keystrokes, but blank submissions are not names.
        // Validate before sanitizing so submission never silently fixes an
        // invalid name; C10 owns trimming and final normalization.
        if (!isValidPlayerName(value_)) {
            error_ = "Enter a valid name (1-15 characters), not only spaces.";
            return false;
        }
        value_ = sanitizePlayerName(value_);
        error_.clear();
        return true;
    }
    const std::string& value() const noexcept { return value_; }
    const std::string& error() const noexcept { return error_; }
private:
    std::string value_;
    std::string error_;
};
} // namespace snake
