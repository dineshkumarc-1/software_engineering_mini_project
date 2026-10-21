#include "snakextreme/Renderer.hpp"
#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

#if defined(_WIN32)
#include <windows.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#elif defined(__linux__)
#include <unistd.h>
#endif

namespace snake {

Renderer::Renderer() {
    std::filesystem::path executableDir;
#if defined(_WIN32)
    // Load the font bundled beside the executable.
    wchar_t executablePath[MAX_PATH];
    const DWORD length = GetModuleFileNameW(
        nullptr, executablePath, MAX_PATH);

    if (length > 0 && length < MAX_PATH) {
        executableDir = std::filesystem::path(executablePath).parent_path();
    }
#elif defined(__APPLE__)
    // _NSGetExecutablePath reports the required buffer size when too small.
    std::uint32_t pathSize = 0;
    (void)_NSGetExecutablePath(nullptr, &pathSize);
    if (pathSize > 0) {
        std::vector<char> executablePath(pathSize);
        if (_NSGetExecutablePath(executablePath.data(), &pathSize) == 0) {
            std::error_code pathError;
            const auto canonicalExecutablePath = std::filesystem::canonical(
                std::filesystem::path(executablePath.data()),
                pathError
            );
            if (!pathError) {
                executableDir = canonicalExecutablePath.parent_path();
            } else {
                executableDir =
                    std::filesystem::path(executablePath.data()).parent_path();
            }
        }
    }
#elif defined(__linux__)
    // readlink does not append a null terminator, so use the returned length.
    std::vector<char> executablePath(4096);
    const auto length = readlink(
        "/proc/self/exe", executablePath.data(), executablePath.size());
    if (length > 0 &&
        static_cast<std::size_t>(length) < executablePath.size()) {
        executableDir = std::filesystem::path(
            std::string(executablePath.data(), static_cast<std::size_t>(length))
        ).parent_path();
    }
#endif

    if (!executableDir.empty()) {
        const auto bundledFontPath =
            executableDir / "assets" / "fonts" / "DejaVuSans.ttf";
        fontLoaded_ = font_.loadFromFile(bundledFontPath.string());
    } else {
        fontLoaded_ = false;
    }

#if defined(SNAKE_ASSET_DIR)
    // Development fallback; packaged builds should use the executable's assets.
    if (!fontLoaded_) {
        const auto sourceTreeFontPath = std::filesystem::path(SNAKE_ASSET_DIR)
            / "fonts" / "dejavu-sans-ttf-2.37" / "ttf" / "DejaVuSans.ttf";
        fontLoaded_ = font_.loadFromFile(sourceTreeFontPath.string());
    }
#endif

}
void Renderer::draw(
    sf::RenderTarget& target,
    Screen screen,
    const GameEngine& gameEngine,
    int currentScore,
    int highScore
) const {
    if (screen == Screen::Exiting) {
        return;
    }

    if (screen == Screen::MainMenu) {
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

    sf::VertexArray grid(sf::Lines);
    const sf::Color gridColor(43, 61, 72);

    for (unsigned x = 0; x <= Width; x += CellSize) {
        grid.append(sf::Vertex(
            {static_cast<float>(x), 0.f}, gridColor));
        grid.append(sf::Vertex(
            {static_cast<float>(x), static_cast<float>(Height)},
            gridColor));
    }

    for (unsigned y = 0; y <= Height; y += CellSize) {
        grid.append(sf::Vertex(
            {0.f, static_cast<float>(y)}, gridColor));
        grid.append(sf::Vertex(
            {static_cast<float>(Width), static_cast<float>(y)},
            gridColor));
    }

    target.draw(grid);

    // Draw gameplay objects first. The HUD is intentionally rendered last.
    if (screen == Screen::Gameplay && gameEngine.hasFood()) {
        const auto food = gameEngine.food();

        sf::CircleShape foodShape(
            static_cast<float>(CellSize) / 2.f
        );
        foodShape.setFillColor(sf::Color(255, 90, 90));
        foodShape.setPosition(
            static_cast<float>(food.x * static_cast<int>(CellSize)),
            static_cast<float>(food.y * static_cast<int>(CellSize))
        );
        target.draw(foodShape);
    }

    if (screen == Screen::Gameplay) {
        const auto& body = gameEngine.snake();

        for (std::size_t i = 0; i < body.size(); ++i) {
            const auto& segment = body[i];

            sf::RectangleShape segmentShape({
                static_cast<float>(CellSize - 2),
                static_cast<float>(CellSize - 2)
            });

            segmentShape.setPosition(
                static_cast<float>(segment.x * static_cast<int>(CellSize) + 1),
                static_cast<float>(segment.y * static_cast<int>(CellSize) + 1)
            );

            segmentShape.setFillColor(
                i == 0
                    ? sf::Color(63, 215, 155)
                    : sf::Color(35, 160, 115)
            );

            target.draw(segmentShape);
        }
    }

    if (screen == Screen::Paused) {
        sf::RectangleShape overlay({
            static_cast<float>(Width),
            static_cast<float>(Height)
        });
        overlay.setFillColor(sf::Color(0, 0, 0, 165));
        target.draw(overlay);

        sf::RectangleShape bar({24.f, 96.f});
        bar.setFillColor(sf::Color(63, 215, 155));
        bar.setPosition(414.f, 252.f);
        target.draw(bar);
        bar.setPosition(462.f, 252.f);
        target.draw(bar);
    }

    // Opaque dark backing keeps the score legible over the grid and gameplay.
    if (fontLoaded_) {
        sf::RectangleShape hudBackground({390.f, 42.f});
        hudBackground.setPosition(10.f, 8.f);
        hudBackground.setFillColor(sf::Color(8, 14, 20, 235));
        hudBackground.setOutlineColor(sf::Color(63, 215, 155));
        hudBackground.setOutlineThickness(1.f);
        target.draw(hudBackground);

        sf::Text scoreText;
        scoreText.setFont(font_);
        scoreText.setCharacterSize(22);
        scoreText.setFillColor(sf::Color(255, 255, 255));
        scoreText.setPosition(20.f, 13.f);
        scoreText.setString(
            "Score: " + std::to_string(currentScore) +
            "    High Score: " + std::to_string(highScore)
        );
        target.draw(scoreText);
    }
}

} // namespace snake
