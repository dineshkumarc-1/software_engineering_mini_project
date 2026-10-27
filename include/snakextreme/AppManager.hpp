
#pragma once

#include "snakextreme/AppState.hpp"
#include "snakextreme/AudioService.hpp"
#include "snakextreme/NameEntry.hpp"
#include "snakextreme/SettingsRepository.hpp"
#include "snakextreme/GameplayController.hpp"
#include "snakextreme/MenuSelection.hpp"
#include "snakextreme/ModeManager.hpp"
#include "snakextreme/Renderer.hpp"

#include <SFML/Graphics.hpp>

#include <string>

namespace snake {

class AppManager {
public:
    AppManager();
    int run();

private:
    void pollEvents();
    void handleKey(sf::Keyboard::Key key);
    void select(MenuInput input);
    void syncAudio(bool previewSfx = false);
    void handleSettingsKey(sf::Keyboard::Key key);
    void activateSettings();
    void clickSettings(int x, int y);
    void startGame();
    void dispatch(Action action);
    void update(sf::Time step);
    void refreshTitle();
    std::string titleText() const;
    FrameView frame() const;

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
