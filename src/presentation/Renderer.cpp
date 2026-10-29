#include "snakextreme/Renderer.hpp"
#include "snakextreme/LeaderboardTable.hpp"
#include "snakextreme/UiHitTest.hpp"
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

#include "snakextreme/GameConfig.hpp"
#include "snakextreme/GameplayController.hpp"

#include <algorithm>
#include <array>
#include <chrono>

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
            executableDir =
                std::filesystem::path(executablePath.data()).parent_path();
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

namespace {

// Colours shared by the screens.
const sf::Color Accent(63, 215, 155);
const sf::Color PanelFill(23, 39, 49);
const sf::Color PanelEdge(60, 80, 92);
const sf::Color TextMain(235, 242, 245);
const sf::Color TextDim(150, 170, 180);
const sf::Color Danger(255, 110, 110);
const sf::Color Warning(255, 170, 60);
const sf::Color ObstacleFill(190, 135, 70);
const sf::Color ObstacleEdge(110, 72, 34);
// Power-ups: one colour per type, all drawn as diamonds (SAD 4.5).
const sf::Color SpeedColour(70, 170, 255);
const sf::Color MultiplierColour(255, 205, 60);
const sf::Color ShrinkColour(200, 120, 255);
const sf::Color IconColour(15, 23, 32);

sf::Color powerUpColour(PowerUpType type) {
    switch (type) {
    case PowerUpType::Speed:
        return SpeedColour;
    case PowerUpType::Multiplier:
        return MultiplierColour;
    case PowerUpType::Shrink:
        return ShrinkColour;
    }
    return SpeedColour;
}

sf::Color withAlpha(sf::Color colour, sf::Uint8 alpha) {
    colour.a = alpha;
    return colour;
}

// UTF-8 for the symbols the bundled DejaVu Sans font provides.
const char* const ArrowLeft = "\xE2\x86\x90";
const char* const ArrowRight = "\xE2\x86\x92";
const char* const ArrowUp = "\xE2\x86\x91";
const char* const ArrowDown = "\xE2\x86\x93";
const char* const Dot = "\xC2\xB7";

constexpr float Cell = static_cast<float>(Renderer::CellSize);

sf::FloatRect cellRect(GridPosition cell) {
    return {static_cast<float>(cell.x) * Cell, static_cast<float>(cell.y) * Cell,
            Cell, Cell};
}

// Vertical middle of capital letters at the label's size. Centring on this
// instead of each label's own bounds keeps labels with and without
// descenders ("Easy", "Medium") on the same line.
float capMiddle(const sf::Text& label) {
    if (const sf::Font* font = label.getFont()) {
        const sf::Text reference("H", *font, label.getCharacterSize());
        const sf::FloatRect cap = reference.getLocalBounds();
        return cap.top + cap.height / 2.f;
    }
    const sf::FloatRect bounds = label.getLocalBounds();
    return bounds.top + bounds.height / 2.f;
}

void centreOn(sf::Text& label, float x, float y) {
    const sf::FloatRect bounds = label.getLocalBounds();
    label.setOrigin(bounds.left + bounds.width / 2.f, capMiddle(label));
    label.setPosition(x, y);
}

// True when a snake segment, the food, an obstacle or a power-up lies under
// `area`, so the HUD can fade there instead of hiding the board.
bool coversGameplay(const sf::FloatRect& area, const GameEngine& engine,
                    const std::optional<PowerUp>& powerUp) {
    const auto under = [&area](GridPosition cell) { return area.intersects(cellRect(cell)); };
    const auto& body = engine.snake();
    const auto& obstacles = engine.obstacles();
    return (powerUp && under(powerUp->cell)) ||
           std::any_of(body.begin(), body.end(), under) ||
           std::any_of(obstacles.begin(), obstacles.end(), under) ||
           (engine.hasFood() && under(engine.food()));
}

std::string modeDescription(GameMode mode) {
    switch (mode) {
    case GameMode::Classic:
        return "No time limit, no obstacles " + std::string(Dot) + " 10 points per food";
    case GameMode::Timed:
        return "Score as much as you can in 60 seconds " + std::string(Dot) +
               " 10 points per food";
    case GameMode::Obstacle:
        return "8 fixed obstacles end the game on contact " + std::string(Dot) +
               " 15 points per food";
    }
    return {};
}

std::string speedDescription(Difficulty difficulty) {
    return "The snake moves one cell every " +
           std::to_string(tickInterval(difficulty).count()) + " ms";
}

} // namespace

sf::Text Renderer::text(const std::string& utf8, unsigned size, sf::Color color) const {
    sf::Text label;
    label.setFont(font_);
    label.setString(sf::String::fromUtf8(utf8.begin(), utf8.end()));
    label.setCharacterSize(size);
    label.setFillColor(color);
    return label;
}

sf::Color Renderer::background(Theme theme) {
    if (theme == Theme::Neon) return sf::Color(14, 8, 32);
    if (theme == Theme::Retro) return sf::Color(29, 40, 24);
    return sf::Color(15, 23, 32);
}

void Renderer::draw(sf::RenderTarget& target, const FrameView& frame) const {
    if (frame.screen == Screen::Exiting) {
        return;
    }

    if (frame.screen == Screen::Settings) {
        drawSettings(target, frame);
        return;
    }
    if (frame.screen == Screen::MainMenu || frame.screen == Screen::NameEntry ||
        frame.screen == Screen::Leaderboard) {
        drawNavigation(target, frame);
        drawFooter(target, frame.screen);
        return;
    }
    if (frame.screen == Screen::ModeSelection) {
        drawMenu(target, frame);
        drawFooter(target, frame.screen);
        return;
    }

    if (frame.engine != nullptr) {
        drawBoard(target, *frame.engine, frame.settings.theme);
    }

    if (frame.engine != nullptr && frame.powerUp) {
        drawPowerUp(target, *frame.powerUp);
    }

    if (frame.engine != nullptr && frame.powerUp) {
        drawPowerUp(target, *frame.powerUp);
    }

    if (frame.screen == Screen::Paused) {
        drawPaused(target);
    } else if (frame.gameOverReason != GameOverReason::None) {
        drawGameOver(target, frame);
    }

    // The HUD is drawn last so it stays readable over every overlay.
    drawHud(target, frame);
    drawFooter(target, frame.screen);
}

void Renderer::drawFooter(sf::RenderTarget& target, Screen screen) const {
    for (const auto& button : footerButtons(screen)) {
        const auto& area = button.area;
        sf::RectangleShape box({static_cast<float>(area.width), static_cast<float>(area.height)});
        box.setPosition(static_cast<float>(area.left), static_cast<float>(area.top));
        box.setFillColor(PanelFill); box.setOutlineColor(Accent); box.setOutlineThickness(1.f);
        target.draw(box);
        if (fontLoaded_) {
            auto label = text(button.label, 20, TextMain);
            centreOn(label, static_cast<float>(area.left) + static_cast<float>(area.width) / 2.f,
                     static_cast<float>(area.top) + static_cast<float>(area.height) / 2.f);
            target.draw(label);
        }
    }
}

void Renderer::drawNavigation(sf::RenderTarget& target, const FrameView& frame) const {
    if (!fontLoaded_) return; // Window title remains the text fallback.
    const auto label = [&](const std::string& value, unsigned size, float y, sf::Color color) {
        auto item = text(value, size, color);
        centreOn(item, 450.f, y);
        target.draw(item);
    };
    label("SnakeXtreme", 46, 78.f, Accent);
    if (frame.screen == Screen::MainMenu) {
        const char* entries[] = {"New Game", "Leaderboard", "Settings", "Exit"};
        for (std::size_t i = 0; i < 4; ++i) {
            const float top = 190.f + 60.f * static_cast<float>(i);
            sf::RectangleShape box({380.f, 48.f});
            box.setPosition(260.f, top);
            box.setFillColor(i == frame.menuIndex ? Accent : PanelFill);
            target.draw(box);
            label(entries[i], 24, top + 24.f, i == frame.menuIndex ? sf::Color(15, 23, 32) : TextMain);
        }
        label("Up/Down or W/S: select   Enter / click: confirm   Esc: exit", 17, 500.f, TextDim);
    } else if (frame.screen == Screen::NameEntry) {
        label("Enter your display name", 28, 190.f, TextMain);
        label(frame.playerName + "_", 32, 275.f, Accent);
        label(std::to_string(frame.playerName.size()) + "/15", 18, 320.f, TextDim);
        label(frame.nameError, 18, 380.f, Danger);
        label("Type name   Backspace: delete   Enter: continue   Esc: back", 17, 500.f, TextDim);
    } else {
        label("Leaderboard - Top 10", 28, 138.f, TextMain);
        const auto cell = [&](const std::string& value, const LeaderboardColumn& column,
                              float y, sf::Color color) {
            auto item = text(value, 17, color);
            // A legal 15-character name (including wide W glyphs) and a maximum
            // integer score must fit without overlapping the adjacent column.
            while (item.getLocalBounds().width > column.width &&
                   item.getCharacterSize() > 12U) {
                item.setCharacterSize(item.getCharacterSize() - 1U);
            }
            item.setPosition(column.left - item.getLocalBounds().left, y);
            target.draw(item);
        };
        for (const auto& column : LeaderboardColumns) {
            cell(std::string(column.heading), column, 165.f, Accent);
        }
        for (std::size_t i = 0; i < frame.leaderboard.size(); ++i) {
            const float y = 194.f + static_cast<float>(i) * 22.f;
            const auto row = leaderboardRow(i + 1U, frame.leaderboard[i]);
            for (std::size_t j = 0; j < row.size(); ++j) {
                cell(row[j], LeaderboardColumns[j], y, j == 0U ? TextDim : TextMain);
            }
        }
        if (frame.leaderboard.empty()) label("No scores yet", 22, 280.f, TextDim);
        // Constant warning text is deliberately bounded to the available row.
        if (!frame.leaderboardMessage.empty()) label("Leaderboard unavailable: empty table shown", 17, 420.f, Warning);
        label("Esc / Back: main menu", 18, 500.f, TextDim);
    }
}

void Renderer::drawSettings(sf::RenderTarget& target, const FrameView& frame) const {
    if (!fontLoaded_) return;
    auto heading = text("Settings", 34, Accent); centreOn(heading, 450.f, 65.f); target.draw(heading);
    const char* names[] = {"Move Up", "Move Down", "Move Left", "Move Right", "Music", "SFX",
                           "Theme", "Restore defaults", "Save & back", "Cancel"};
    for (std::size_t i = 0; i < 10; ++i) {
        const float top = 120.f + 35.f * static_cast<float>(i);
        sf::RectangleShape row({580.f, 32.f}); row.setPosition(160.f, top);
        row.setFillColor(i == frame.settingsRow ? sf::Color(45, 65, 75) : PanelFill); target.draw(row);
        auto name = text(names[i], 19, i == frame.settingsRow ? Accent : TextMain);
        name.setPosition(175.f, top + 3.f); target.draw(name);
        std::string value;
        if (i < 4) value = frame.capturingKey && frame.settingsRow == i ? "Press key..." : keyName(frame.settings.keys[i]);
        if (i == 6) value = std::string(themeName(frame.settings.theme)) + "  < >";
        if (i == 4 || i == 5) {
            const int volume = i == 4 ? frame.settings.musicVolume : frame.settings.sfxVolume;
            sf::RectangleShape track({300.f, 5.f}); track.setPosition(400.f, top + 16.f);
            track.setFillColor(PanelEdge); target.draw(track);
            sf::RectangleShape fill({static_cast<float>(volume) * 3.f, 5.f}); fill.setPosition(track.getPosition());
            fill.setFillColor(Accent); target.draw(fill);
            sf::CircleShape knob(6.f); knob.setPosition(394.f + static_cast<float>(volume) * 3.f, top + 12.f);
            knob.setFillColor(TextMain); target.draw(knob);
            auto number = text(std::to_string(volume), 16, TextMain); number.setPosition(345.f, top + 4.f); target.draw(number);
        } else {
            auto item = text(value, 18, TextMain); item.setPosition(400.f, top + 3.f); target.draw(item);
        }
    }
    auto message = text(frame.settingsMessage, 15, Warning); centreOn(message, 450.f, 490.f); target.draw(message);
    auto help = text("Up/Down: row   Left/Right: adjust   Enter / click: edit   Esc: cancel", 16, TextDim);
    centreOn(help, 450.f, 525.f); target.draw(help);
    auto note = text(frame.audioAvailable
        ? "Live audio preview. F5: test SFX. Save applies; Cancel restores. WASD always enabled."
        : "Audio unavailable: silent fallback. Values still save. WASD always enabled.", 15, TextDim);
    centreOn(note, 450.f, 555.f); target.draw(note);
}

void Renderer::drawMenu(sf::RenderTarget& target, const FrameView& frame) const {
    const auto modeIndex = [&](GameMode mode) {
        for (std::size_t i = 0; i < AllGameModes.size(); ++i) {
            if (AllGameModes[i] == mode) {
                return i;
            }
        }
        return std::size_t{0};
    };
    const auto difficultyIndex = [&](Difficulty difficulty) {
        for (std::size_t i = 0; i < AllDifficulties.size(); ++i) {
            if (AllDifficulties[i] == difficulty) {
                return i;
            }
        }
        return std::size_t{0};
    };

    if (!fontLoaded_) {
        // Font-free fallback: the window title names the selection; the
        // squares show which mode (top) and difficulty (bottom) are chosen.
        sf::RectangleShape panel({540.f, 240.f});
        panel.setPosition(180.f, 180.f);
        panel.setFillColor(PanelFill);
        panel.setOutlineColor(Accent);
        panel.setOutlineThickness(2.f);
        target.draw(panel);

        sf::CircleShape play(48.f, 3);
        play.setOrigin(48.f, 48.f);
        play.setPosition(450.f, 300.f);
        play.setRotation(90.f);
        play.setFillColor(Accent);
        target.draw(play);

        for (std::size_t row = 0; row < 2; ++row) {
            const std::size_t chosen = row == 0 ? modeIndex(frame.selected.mode)
                                                : difficultyIndex(frame.selected.difficulty);
            for (std::size_t i = 0; i < 3; ++i) {
                sf::RectangleShape marker({20.f, 20.f});
                marker.setPosition(410.f + 30.f * static_cast<float>(i),
                                   row == 0 ? 200.f : 380.f);
                marker.setFillColor(i == chosen ? Accent : PanelFill);
                marker.setOutlineColor(Accent);
                marker.setOutlineThickness(2.f);
                target.draw(marker);
            }
        }
        return;
    }

    sf::Text title = text("SnakeXtreme", 46, Accent);
    centreOn(title, 450.f, 78.f);
    target.draw(title);

    sf::Text subtitle = text("Choose a mode and difficulty; click Start or press Enter", 18, TextDim);
    centreOn(subtitle, 450.f, 126.f);
    target.draw(subtitle);

    // One row of three options; the selected one is filled.
    const auto drawOptions = [&](const char* heading, float top,
                                 const std::array<std::string, 3>& labels,
                                 std::size_t chosen) {
        sf::Text label = text(heading, 15, TextDim);
        centreOn(label, 450.f, top - 20.f);
        target.draw(label);

        for (std::size_t i = 0; i < labels.size(); ++i) {
            const float left = 160.f + 200.f * static_cast<float>(i);
            const bool selected = i == chosen;

            sf::RectangleShape box({180.f, 48.f});
            box.setPosition(left, top);
            box.setFillColor(selected ? Accent : PanelFill);
            box.setOutlineColor(selected ? Accent : PanelEdge);
            box.setOutlineThickness(2.f);
            target.draw(box);

            sf::Text option = text(labels[i], 22,
                                   selected ? sf::Color(15, 23, 32) : TextMain);
            centreOn(option, left + 90.f, top + 24.f);
            target.draw(option);
        }
    };

    drawOptions("MODE", 200.f,
                {std::string(toString(GameMode::Classic)),
                 std::string(toString(GameMode::Timed)),
                 std::string(toString(GameMode::Obstacle))},
                modeIndex(frame.selected.mode));

    sf::Text modeInfo = text(modeDescription(frame.selected.mode), 16, TextDim);
    centreOn(modeInfo, 450.f, 270.f);
    target.draw(modeInfo);

    drawOptions("DIFFICULTY", 338.f,
                {std::string(toString(Difficulty::Easy)),
                 std::string(toString(Difficulty::Medium)),
                 std::string(toString(Difficulty::Hard))},
                difficultyIndex(frame.selected.difficulty));

    sf::Text speedInfo = text(speedDescription(frame.selected.difficulty), 16, TextDim);
    centreOn(speedInfo, 450.f, 408.f);
    target.draw(speedInfo);

    const std::string controls =
        std::string(ArrowLeft) + " " + ArrowRight + "  mode       " +
        ArrowUp + " " + ArrowDown + "  difficulty       Enter  start       Esc  back";
    sf::Text hint = text(controls, 18, TextMain);
    centreOn(hint, 450.f, 510.f);
    target.draw(hint);

    if (frame.highScore > 0) {
        sf::Text best = text("Best score this session: " + std::to_string(frame.highScore),
                             16, TextDim);
        centreOn(best, 450.f, 548.f);
        target.draw(best);
    }

    if (!frame.menuError.empty()) {
        sf::Text error = text(frame.menuError, 17, Danger);
        centreOn(error, 450.f, 580.f);
        target.draw(error);
    }
}

void Renderer::drawBoard(sf::RenderTarget& target, const GameEngine& engine, Theme theme) const {
    sf::VertexArray grid(sf::Lines);
    const sf::Color gridColor = theme == Theme::Neon ? sf::Color(85, 40, 115) :
        theme == Theme::Retro ? sf::Color(63, 82, 47) : sf::Color(43, 61, 72);
    const sf::Color headColor = theme == Theme::Neon ? sf::Color(30, 255, 245) :
        theme == Theme::Retro ? sf::Color(190, 210, 100) : Accent;
    const sf::Color bodyColor = theme == Theme::Neon ? sf::Color(220, 45, 230) :
        theme == Theme::Retro ? sf::Color(125, 155, 65) : sf::Color(35, 160, 115);

    for (unsigned x = 0; x <= Width; x += CellSize) {
        grid.append(sf::Vertex({static_cast<float>(x), 0.f}, gridColor));
        grid.append(sf::Vertex(
            {static_cast<float>(x), static_cast<float>(Height)}, gridColor));
    }

    for (unsigned y = 0; y <= Height; y += CellSize) {
        grid.append(sf::Vertex({0.f, static_cast<float>(y)}, gridColor));
        grid.append(sf::Vertex(
            {static_cast<float>(Width), static_cast<float>(y)}, gridColor));
    }

    target.draw(grid);

    // Obstacles: solid brown blocks with a dark inner edge, so they read
    // differently from the round red food and the green snake.
    for (const GridPosition& obstacle : engine.obstacles()) {
        sf::RectangleShape block({Cell - 2.f, Cell - 2.f});
        block.setPosition(static_cast<float>(obstacle.x) * Cell + 1.f,
                          static_cast<float>(obstacle.y) * Cell + 1.f);
        block.setFillColor(ObstacleFill);
        block.setOutlineColor(ObstacleEdge);
        block.setOutlineThickness(-4.f);
        target.draw(block);
    }

    if (engine.hasFood()) {
        const GridPosition food = engine.food();
        sf::CircleShape foodShape(Cell / 2.f);
        foodShape.setFillColor(sf::Color(255, 90, 90));
        foodShape.setPosition(static_cast<float>(food.x) * Cell,
                              static_cast<float>(food.y) * Cell);
        target.draw(foodShape);
    }

    const auto& body = engine.snake();
    for (std::size_t i = 0; i < body.size(); ++i) {
        sf::RectangleShape segment({Cell - 2.f, Cell - 2.f});
        segment.setPosition(static_cast<float>(body[i].x) * Cell + 1.f,
                            static_cast<float>(body[i].y) * Cell + 1.f);
        segment.setFillColor(i == 0 ? headColor : bodyColor);
        target.draw(segment);
    }
}

void Renderer::drawHud(sf::RenderTarget& target, const FrameView& frame) const {
    if (!fontLoaded_ || frame.engine == nullptr) {
        return;
    }

    // Compact panels in the top corners. Each one fades when part of the
    // board is underneath, so the HUD never fully hides the game.
    const auto drawPanel = [&](const sf::FloatRect& area) {
        const bool faded = coversGameplay(area, *frame.engine, frame.powerUp);
        sf::RectangleShape panel({area.width, area.height});
        panel.setPosition(area.left, area.top);
        panel.setFillColor(sf::Color(8, 14, 20, faded ? 110 : 215));
        panel.setOutlineColor(sf::Color(Accent.r, Accent.g, Accent.b, faded ? 110 : 255));
        panel.setOutlineThickness(1.f);
        target.draw(panel);
    };

    sf::Text score = text("Score " + std::to_string(frame.score) + "     High " +
                              std::to_string(frame.highScore),
                          20, TextMain);
    const sf::FloatRect scoreBounds = score.getLocalBounds();
    const sf::FloatRect scoreArea(8.f, 8.f, scoreBounds.width + 24.f, 34.f);
    drawPanel(scoreArea);
    centreOn(score, scoreArea.left + scoreArea.width / 2.f, scoreArea.top + 17.f);
    target.draw(score);

    sf::Text settings = text(std::string(toString(frame.active.mode)) + " " + Dot + " " +
                                 std::string(toString(frame.active.difficulty)),
                             20, TextMain);
    sf::Text countdown;
    float width = settings.getLocalBounds().width + 24.f;
    if (frame.remainingTime) {
        const auto left = *frame.remainingTime;
        const sf::Color colour = left.count() == 0                  ? Danger
                                 : left <= std::chrono::seconds{10} ? Warning
                                                                    : TextMain;
        countdown = text(formatCountdown(left), 20, colour);
        countdown.setStyle(sf::Text::Bold);
        width += countdown.getLocalBounds().width + 18.f;
    }

    const sf::FloatRect settingsArea(static_cast<float>(Width) - 8.f - width, 8.f, width, 34.f);
    drawPanel(settingsArea);

    const sf::FloatRect settingsBounds = settings.getLocalBounds();
    settings.setOrigin(settingsBounds.left, capMiddle(settings));
    settings.setPosition(settingsArea.left + 12.f, settingsArea.top + 17.f);
    target.draw(settings);

    if (frame.remainingTime) {
        const sf::FloatRect countdownBounds = countdown.getLocalBounds();
        countdown.setOrigin(countdownBounds.left + countdownBounds.width,
                            capMiddle(countdown));
        countdown.setPosition(settingsArea.left + settingsArea.width - 12.f,
                              settingsArea.top + 17.f);
        target.draw(countdown);
    }

    // Top centre: the running power-up effect with its seconds left and a
    // bar that empties (SAD 4.5). Hidden once the game is over.
    if (frame.activeEffect && frame.gameOverReason == GameOverReason::None) {
        const ActiveEffect& effect = *frame.activeEffect;
        const sf::Color colour = powerUpColour(effect.type);

        sf::Text label = text(describeEffect(effect), 20, TextMain);
        const float labelWidth = label.getLocalBounds().width;
        const float effectWidth = labelWidth + 50.f;
        const sf::FloatRect effectArea(static_cast<float>(Width) / 2.f - effectWidth / 2.f, 8.f,
                                       effectWidth, 34.f);
        drawPanel(effectArea);

        sf::CircleShape icon(8.f, 4);
        icon.setOrigin(8.f, 8.f);
        icon.setPosition(effectArea.left + 18.f, effectArea.top + 17.f);
        icon.setFillColor(colour);
        target.draw(icon);

        const sf::FloatRect labelBounds = label.getLocalBounds();
        label.setOrigin(labelBounds.left, capMiddle(label));
        label.setPosition(effectArea.left + 34.f, effectArea.top + 17.f);
        target.draw(label);

        const auto total = std::chrono::duration_cast<std::chrono::microseconds>(
            effectFor(effect.type).duration());
        if (total.count() > 0) {
            float fraction = static_cast<float>(effect.remaining.count()) /
                             static_cast<float>(total.count());
            fraction = fraction < 0.f ? 0.f : (fraction > 1.f ? 1.f : fraction);
            sf::RectangleShape bar({(effectArea.width - 8.f) * fraction, 3.f});
            bar.setPosition(effectArea.left + 4.f, effectArea.top + effectArea.height - 6.f);
            bar.setFillColor(colour);
            target.draw(bar);
        }
    }
}

void Renderer::drawPaused(sf::RenderTarget& target) const {
    sf::RectangleShape overlay({static_cast<float>(Width), static_cast<float>(Height)});
    overlay.setFillColor(sf::Color(0, 0, 0, 150));
    target.draw(overlay);

    sf::RectangleShape bar({24.f, 96.f});
    bar.setFillColor(Accent);
    bar.setPosition(414.f, 252.f);
    target.draw(bar);
    bar.setPosition(462.f, 252.f);
    target.draw(bar);

    if (fontLoaded_) {
        sf::Text paused = text("Paused", 34, TextMain);
        centreOn(paused, 450.f, 210.f);
        target.draw(paused);

        sf::Text hint = text("Esc / Enter  resume       M  menu", 18, TextDim);
        centreOn(hint, 450.f, 390.f);
        target.draw(hint);
    }
}

void Renderer::drawGameOver(sf::RenderTarget& target, const FrameView& frame) const {
    // Light dimming keeps the final board visible behind the message.
    sf::RectangleShape overlay({static_cast<float>(Width), static_cast<float>(Height)});
    overlay.setFillColor(sf::Color(0, 0, 0, 110));
    target.draw(overlay);

    if (!fontLoaded_) {
        sf::RectangleShape border({static_cast<float>(Width) - 8.f,
                                   static_cast<float>(Height) - 8.f});
        border.setPosition(4.f, 4.f);
        border.setFillColor(sf::Color::Transparent);
        border.setOutlineColor(Danger);
        border.setOutlineThickness(4.f);
        target.draw(border);
        return;
    }

    // Put the message in the half of the board away from the snake's head,
    // so the spot where the game ended stays visible.
    const bool headInTopHalf = frame.engine != nullptr && !frame.engine->snake().empty() &&
                               frame.engine->snake().front().y < snake::config::GridRows / 2;
    const float top = headInTopHalf ? 360.f : 70.f;

    sf::RectangleShape panel({460.f, 200.f});
    panel.setPosition(220.f, top);
    panel.setFillColor(sf::Color(PanelFill.r, PanelFill.g, PanelFill.b, 235));
    panel.setOutlineColor(Danger);
    panel.setOutlineThickness(2.f);
    target.draw(panel);

    sf::Text heading = text(frame.gameOverReason == GameOverReason::BOARD_FULL ? "You win"
                                                                                : "Game over",
                            34, Danger);
    centreOn(heading, 450.f, top + 38.f);
    target.draw(heading);

    if (!frame.leaderboardMessage.empty()) {
        auto saved = text(frame.leaderboardMessage, 16, Warning);
        centreOn(saved, 450.f, 505.f); target.draw(saved);
    }
    sf::Text reason = text(std::string(describeGameOver(frame.gameOverReason)), 22, TextMain);
    centreOn(reason, 450.f, top + 84.f);
    target.draw(reason);

    sf::Text finalScore = text("Final score " + std::to_string(frame.score), 20, TextMain);
    centreOn(finalScore, 450.f, top + 122.f);
    target.draw(finalScore);

    sf::Text hint = text("Enter  play again       M  menu", 17, TextDim);
    centreOn(hint, 450.f, top + 164.f);
    target.draw(hint);
}

void Renderer::drawPowerUp(sf::RenderTarget& target, const PowerUp& item) const {
    // Blinks during its last 1.5 s, so the player sees it is about to go.
    const bool dimmed = item.remaining < std::chrono::milliseconds{1500} &&
                        (item.remaining.count() / 125000) % 2 == 0;
    const sf::Uint8 alpha = static_cast<sf::Uint8>(dimmed ? 90 : 255);

    const float centreX = static_cast<float>(item.cell.x) * Cell + Cell / 2.f;
    const float centreY = static_cast<float>(item.cell.y) * Cell + Cell / 2.f;

    // A diamond, so power-ups differ by shape from the round food and the
    // square obstacles and snake, not only by colour (SAD 4.5).
    const float radius = Cell / 2.f - 2.f;
    sf::CircleShape diamond(radius, 4);
    diamond.setOrigin(radius, radius);
    diamond.setPosition(centreX, centreY);
    diamond.setFillColor(withAlpha(powerUpColour(item.type), alpha));
    diamond.setOutlineColor(sf::Color(255, 255, 255, alpha));
    diamond.setOutlineThickness(1.5f);
    target.draw(diamond);

    // A small dark sign on top: >> for Speed, x2 for Multiplier, - for Shrink.
    const sf::Color sign = withAlpha(IconColour, alpha);
    switch (item.type) {
    case PowerUpType::Speed: {
        sf::CircleShape arrow(4.f, 3);
        arrow.setOrigin(4.f, 4.f);
        arrow.setRotation(90.f);
        arrow.setFillColor(sign);
        arrow.setPosition(centreX - 3.f, centreY);
        target.draw(arrow);
        arrow.setPosition(centreX + 4.f, centreY);
        target.draw(arrow);
        break;
    }
    case PowerUpType::Multiplier:
        if (fontLoaded_) {
            sf::Text times = text("x2", 12, sign);
            times.setStyle(sf::Text::Bold);
            centreOn(times, centreX, centreY);
            target.draw(times);
        } else {
            sf::CircleShape dot(3.5f);
            dot.setOrigin(3.5f, 3.5f);
            dot.setPosition(centreX, centreY);
            dot.setFillColor(sign);
            target.draw(dot);
        }
        break;
    case PowerUpType::Shrink: {
        sf::RectangleShape minus({12.f, 3.f});
        minus.setOrigin(6.f, 1.5f);
        minus.setPosition(centreX, centreY);
        minus.setFillColor(sign);
        target.draw(minus);
        break;
    }
    }
}

} // namespace snake
