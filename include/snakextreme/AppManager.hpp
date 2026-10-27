
#pragma once

#include "snakextreme/AppState.hpp"
#include "snakextreme/AudioService.hpp"
#include "snakextreme/PerformanceProbe.hpp"
#include "snakextreme/NameEntry.hpp"
#include "snakextreme/SettingsRepository.hpp"
#include "snakextreme/GameplayController.hpp"
#include "snakextreme/MenuSelection.hpp"
#include "snakextreme/ModeManager.hpp"
#include "snakextreme/Renderer.hpp"

#include <SFML/Graphics.hpp>

#include <string>
#include <chrono>
#include <optional>
#include <cstdint>

namespace snake {

class AppManager {
public:
    explicit AppManager(PerformanceProbe* probe = nullptr,
        std::chrono::steady_clock::time_point mainEntry = std::chrono::steady_clock::now(),
        std::optional<int> probeSeconds = std::nullopt);
    int run();

private:
    std::int64_t probeNow() const;
    ProbeContext probeContext() const;
    void pollEvents();
    void handleKey(sf::Keyboard::Key key);
    void select(MenuInput input);
    void syncAudio(bool previewSfx = false);
    void handleSettingsKey(sf::Keyboard::Key key);
    void activateSettings();
    void clickSettings(int x, int y);
    void clickNavigation(int x, int y);
    void startGame();
    void dispatch(Action action);
    void update(sf::Time step);
    void refreshTitle();
    std::string titleText() const;
    FrameView frame() const;

    PerformanceProbe* probe_{nullptr};
    std::chrono::steady_clock::time_point probeStart_;
    std::optional<int> probeSeconds_;
    std::int64_t inputDequeuedUs_{0};
    AudioService audio_;
    SettingsRepository settingsRepository_;
    Settings settings_;
    SettingsEditor settingsEditor_;
    std::string settingsNotice_;
    bool draggingVolume_{false};
    NameEntry nameEntry_;
    std::size_t menuIndex_{0};
    sf::RenderWindow window_;
    AppState state_;
    Renderer renderer_;

    // Main-menu choice; Classic / Medium by default (FR-008, FR-009).
    SessionConfig selectedConfig_{};
    // Engine, ModeManager and score for the current game.
    GameplayController gameplay_;
    // Shown on the menu when a game could not start.
    std::string menuError_;
    std::string title_;
};

} // namespace snake
