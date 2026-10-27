#pragma once

#include <array>
#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

namespace snake {

// SFML 2.x key codes are the on-disk contract; no SFML dependency here.
// Order: Up, Down, Left, Right. WASD aliases remain enabled (SRS Addendum).
enum class Theme { Classic, Neon, Retro };
struct Settings {
    std::array<int, 4> keys{73, 74, 71, 72};
    int musicVolume{70};
    int sfxVolume{70};
    Theme theme{Theme::Classic};
};
bool operator==(const Settings& a, const Settings& b) noexcept;
bool isMovementKey(int code) noexcept;
bool validSettings(const Settings& settings) noexcept;
std::string keyName(int code);
std::string_view themeName(Theme theme) noexcept;
std::optional<std::size_t> movementForKey(const Settings& settings, int code) noexcept;

// Transactional editor: the app applies settings only after a successful save.
class SettingsEditor {
public:
    void open(const Settings& settings);
    const Settings& value() const noexcept { return value_; }
    std::size_t row() const noexcept { return row_; }
    bool capturing() const noexcept { return capturing_; }
    const std::string& message() const noexcept { return message_; }
    void move(int delta);
    void selectRow(std::size_t row);
    void adjust(int delta);
    void setVolume(int value);
    void activate();
    bool bind(int key);
    void cancelCapture() noexcept { capturing_ = false; }
    void error(std::string message) { message_ = std::move(message); }
private:
    Settings value_{};
    std::size_t row_{0};
    bool capturing_{false};
    std::string message_;
};
} // namespace snake
