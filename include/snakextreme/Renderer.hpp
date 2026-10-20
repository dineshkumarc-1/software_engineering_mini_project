
#pragma once

#include "snakextreme/AppState.hpp"
#include "snakextreme/GameEngine.hpp"
#include <SFML/Graphics.hpp>

namespace snake {

class Renderer {
public:
    static constexpr unsigned Width = 900;
    static constexpr unsigned Height = 600;

    void draw(sf::RenderTarget& target,
              Screen screen,
              const GameEngine& engine) const;
};

} // namespace snake
