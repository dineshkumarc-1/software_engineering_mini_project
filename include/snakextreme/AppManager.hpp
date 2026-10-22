
#pragma once

#include "snakextreme/AppState.hpp"
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
    void startGame();
    void dispatch(Action action);
    void update(sf::Time step);
    void refreshTitle();
    std::string titleText() const;
    FrameView frame() const;

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
