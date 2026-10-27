#include "snakextreme/Settings.hpp"
#include <algorithm>
#include <utility>

namespace snake {
bool operator==(const Settings& a, const Settings& b) noexcept {
    return a.keys == b.keys && a.musicVolume == b.musicVolume &&
           a.sfxVolume == b.sfxVolume && a.theme == b.theme;
}
bool isMovementKey(int code) noexcept {
    // Letters except M (return to menu), plus arrow keys. Other control keys
    // are deliberately reserved. Contradictory permanent WASD aliases rejected
    // by validSettings/bind rather than silently shadowed.
    return (code >= 0 && code <= 25 && code != 12) || (code >= 71 && code <= 74);
}
bool validSettings(const Settings& s) noexcept {
    if (s.musicVolume < 0 || s.musicVolume > 100 || s.sfxVolume < 0 || s.sfxVolume > 100 ||
        (s.theme != Theme::Classic && s.theme != Theme::Neon && s.theme != Theme::Retro)) return false;
    constexpr std::array<int, 4> aliases{22, 18, 0, 3}; // W,S,A,D
    for (std::size_t i = 0; i < s.keys.size(); ++i) {
        if (!isMovementKey(s.keys[i])) return false;
        for (std::size_t j = 0; j < s.keys.size(); ++j) {
            if (i != j && (s.keys[i] == s.keys[j] || s.keys[i] == aliases[j])) return false;
        }
    }
    return true;
}
std::string keyName(int code) {
    if (code >= 0 && code <= 25) return std::string(1, static_cast<char>('A' + code));
    switch (code) { case 71: return "Left"; case 72: return "Right";
        case 73: return "Up"; case 74: return "Down"; default: return "Invalid"; }
}
std::string_view themeName(Theme t) noexcept {
    switch (t) { case Theme::Classic: return "Classic"; case Theme::Neon: return "Neon";
        case Theme::Retro: return "Retro"; } return "Invalid";
}
std::optional<std::size_t> movementForKey(const Settings& s, int code) noexcept {
    constexpr std::array<int, 4> aliases{22, 18, 0, 3};
    for (std::size_t i = 0; i < aliases.size(); ++i)
        if (code == aliases[i] || code == s.keys[i]) return i;
    return std::nullopt;
}
void SettingsEditor::open(const Settings& s) {
    value_ = s; row_ = 0; capturing_ = false; message_.clear();
}
void SettingsEditor::move(int delta) {
    if (!capturing_) row_ = static_cast<std::size_t>((static_cast<int>(row_) + delta % 10 + 10) % 10);
}
void SettingsEditor::selectRow(std::size_t row) {
    if (!capturing_ && row < 10) row_ = row;
}
void SettingsEditor::adjust(int delta) {
    if (capturing_) return;
    if (row_ == 4 || row_ == 5) {
        int& volume = row_ == 4 ? value_.musicVolume : value_.sfxVolume;
        volume = std::clamp(volume + std::clamp(delta, -100, 100), 0, 100);
    } else if (row_ == 6) {
        value_.theme = static_cast<Theme>((static_cast<int>(value_.theme) + delta % 3 + 3) % 3);
    }
}
void SettingsEditor::setVolume(int value) {
    if (capturing_) return;
    if (row_ == 4) value_.musicVolume = std::clamp(value, 0, 100);
    if (row_ == 5) value_.sfxVolume = std::clamp(value, 0, 100);
}
void SettingsEditor::activate() {
    if (row_ < 4) { capturing_ = true; message_ = "Press a letter or arrow; Esc cancels. M is reserved."; }
    else if (row_ == 6) adjust(1);
    else if (row_ == 7) { value_ = Settings{}; message_ = "Defaults restored in draft. Save to apply."; }
}
bool SettingsEditor::bind(int key) {
    if (!capturing_ || row_ >= 4) return false;
    auto candidate = value_;
    candidate.keys[row_] = key;
    if (!validSettings(candidate)) {
        message_ = "Key reserved, duplicate, or conflicts with another WASD direction.";
        return false;
    }
    value_ = candidate; capturing_ = false; message_ = "Binding updated. Save to apply.";
    return true;
}
} // namespace snake
