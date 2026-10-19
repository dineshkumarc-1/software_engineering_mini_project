#include "snakextreme/Renderer.hpp"

namespace snake {
void Renderer::draw(sf::RenderTarget& target, Screen screen) const {
    if (screen == Screen::Exiting) return;
    if (screen == Screen::MainMenu) {
        // Decorative menu placeholder: no assets or system-font dependency.
        sf::RectangleShape panel({540.f, 240.f});
        panel.setPosition(180.f, 180.f);
        panel.setFillColor(sf::Color(23, 39, 49));
        panel.setOutlineColor(sf::Color(63, 215, 155));
        panel.setOutlineThickness(2.f);
        target.draw(panel);
        sf::CircleShape play(48.f, 3);
        play.setOrigin(48.f, 48.f);
        play.setPosition(450.f, 300.f);
        play.setRotation(90.f);
        play.setFillColor(sf::Color(63, 215, 155));
        target.draw(play);
        return;
    }
    // 30x20 grid preview only; no engine state, snake, or game rules yet.
    sf::VertexArray grid(sf::Lines);
    const sf::Color gridColor(43, 61, 72);
    for (unsigned x = 0; x <= Width; x += 30) {
        grid.append(sf::Vertex({static_cast<float>(x), 0.f}, gridColor));
        grid.append(sf::Vertex({static_cast<float>(x), static_cast<float>(Height)}, gridColor));
    }
    for (unsigned y = 0; y <= Height; y += 30) {
        grid.append(sf::Vertex({0.f, static_cast<float>(y)}, gridColor));
        grid.append(sf::Vertex({static_cast<float>(Width), static_cast<float>(y)}, gridColor));
    }
    target.draw(grid);
    if (screen == Screen::Paused) {
        sf::RectangleShape overlay({static_cast<float>(Width), static_cast<float>(Height)});
        overlay.setFillColor(sf::Color(0, 0, 0, 165));
        target.draw(overlay);
        sf::RectangleShape bar({24.f, 96.f});
        bar.setFillColor(sf::Color(63, 215, 155));
        bar.setPosition(414.f, 252.f); target.draw(bar);
        bar.setPosition(462.f, 252.f); target.draw(bar);
    }
}
} // namespace snake
