#pragma once
#include "snakextreme/AppState.hpp"
#include "snakextreme/Renderer.hpp"
#include <SFML/Graphics.hpp>
#include "snakextreme/GameSession.hpp"

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
    GameSession gameSession_;
};
} // namespace snake
