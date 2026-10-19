
#pragma once

#include "snakextreme/AppState.hpp"
#include <SFML/Graphics.hpp>

namespace snake {

class Renderer {
public:
    static constexpr unsigned Width = 900;
    static constexpr unsigned Height = 600;

    Renderer();

    void draw(
        sf::RenderTarget& target,
        Screen screen,
        int currentScore,
        int highScore
    ) const;

private:
    sf::Font font_;
    bool fontLoaded_{false};
};

} // namespace snake
