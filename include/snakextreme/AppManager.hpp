
#pragma once

#include "snakextreme/AppState.hpp"
#include "snakextreme/GameEngine.hpp"
#include "snakextreme/GameSession.hpp"
#include "snakextreme/Renderer.hpp"

#include <SFML/Graphics.hpp>

namespace snake {

class AppManager {
public:
    AppManager();
    int run();

private:
    void pollEvents();
    void dispatch(Action action);
    void update(sf::Time step);
    void updateTitle();

    sf::RenderWindow window_;
    AppState state_;
    Renderer renderer_;
    GameEngine gameEngine_;
    GameSession gameSession_;

    sf::Time movementAccumulator_{sf::Time::Zero};
    GameMode activeMode_{GameMode::Classic};
};

} // namespace snake
