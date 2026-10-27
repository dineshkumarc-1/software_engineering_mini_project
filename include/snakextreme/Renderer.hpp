
#pragma once

#include "snakextreme/AppState.hpp"
#include "snakextreme/GameEngine.hpp"
#include "snakextreme/ModeManager.hpp"
#include <SFML/Graphics.hpp>

#include <chrono>
#include <optional>
#include <string>

namespace snake {

// Everything one frame shows, gathered by AppManager. The renderer only
// draws it; no game rules live here.
struct FrameView {
    Screen screen{Screen::MainMenu};
    const GameEngine* engine{nullptr};
    SessionConfig selected{};  // main-menu choice
    SessionConfig active{};    // settings of the current game
    std::optional<std::chrono::milliseconds> remainingTime; // Timed only
    GameOverReason gameOverReason{GameOverReason::None};
    int score{0};
    int highScore{0};
    std::string menuError;
    std::string playerName;
    std::string nameError;
    std::size_t menuIndex{0};
};

class Renderer {
public:
    static constexpr unsigned Width = 900;
    static constexpr unsigned Height = 600;
    static constexpr unsigned CellSize = 30;

    Renderer();

    void draw(sf::RenderTarget& target, const FrameView& frame) const;

private:
    void drawNavigation(sf::RenderTarget& target, const FrameView& frame) const;
    void drawMenu(sf::RenderTarget& target, const FrameView& frame) const;
    void drawBoard(sf::RenderTarget& target, const GameEngine& engine) const;
    void drawHud(sf::RenderTarget& target, const FrameView& frame) const;
    void drawPaused(sf::RenderTarget& target) const;
    void drawGameOver(sf::RenderTarget& target, const FrameView& frame) const;
    sf::Text text(const std::string& utf8, unsigned size, sf::Color color) const;

    sf::Font font_;
    bool fontLoaded_{false};
};

} // namespace snake
