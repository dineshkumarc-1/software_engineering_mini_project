
#pragma once

#include "snakextreme/AppState.hpp"
#include "snakextreme/GameEngine.hpp"
#include <SFML/Graphics.hpp>

namespace snake {

class Renderer {
public:
    static constexpr unsigned Width = 900;
    static constexpr unsigned Height = 600;
    static constexpr unsigned CellSize = 30;

    Renderer();

    void draw(
        sf::RenderTarget& target,
        Screen screen,
        const GameEngine& gameEngine,
        int currentScore,
        int highScore
    ) const;

private:
    sf::Font font_;
    bool fontLoaded_{false};
};

} // namespace snake
