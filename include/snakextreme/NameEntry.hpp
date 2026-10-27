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
        if (codepoint > 127 || !isValidPlayerName(std::string(1, static_cast<char>(codepoint)))) {
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
        const auto first = value_.find_first_not_of(' ');
        if (first == std::string::npos) {
            error_ = "Enter a name (1-15 characters), not only spaces.";
            return false;
        }
        const auto last = value_.find_last_not_of(' ');
        const std::string trimmed = value_.substr(first, last - first + 1);
        if (!isValidPlayerName(trimmed)) {
            error_ = "Invalid player name.";
            return false;
        }
        value_ = trimmed;
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
