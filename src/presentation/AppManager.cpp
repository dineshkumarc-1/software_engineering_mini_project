
#include "snakextreme/AppManager.hpp"

#include <algorithm>
#include <chrono>

namespace snake {

namespace {

std::string settingsText(const SessionConfig& config) {
    return std::string(toString(config.mode)) + " / " +
           std::string(toString(config.difficulty));
}

} // namespace

AppManager::AppManager()
    : window_(
          sf::VideoMode(Renderer::Width, Renderer::Height),
          "SnakeXtreme",
          sf::Style::Titlebar | sf::Style::Close) {
    window_.setFramerateLimit(60);
    window_.setKeyRepeatEnabled(false);
    refreshTitle();
}

int AppManager::run() {
    sf::Clock clock;
    sf::Time accumulator = sf::Time::Zero;
    const sf::Time fixedStep = sf::seconds(1.f / 60.f);

    while (window_.isOpen()) {
        const sf::Time elapsed = clock.restart();

        pollEvents();

        // Do not update or render after the window closes.
        if (!window_.isOpen()) {
            break;
        }

        // Limit catch-up updates after a pause or system stall.
        accumulator += std::min(elapsed, sf::milliseconds(250));

        while (accumulator >= fixedStep) {
            update(fixedStep);
            accumulator -= fixedStep;
        }

        refreshTitle();

        window_.clear(sf::Color(15, 23, 32));
        renderer_.draw(window_, frame());
        window_.display();
    }

    return 0;
}

void AppManager::pollEvents() {
    sf::Event event{};

    while (window_.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            dispatch(Action::Quit);
            return;
        }

        if (event.type == sf::Event::LostFocus &&
            state_.screen() == Screen::Gameplay) {
            dispatch(Action::Pause);
        }

        if (event.type == sf::Event::KeyPressed) {
            handleKey(event.key.code);
        }

        if (!window_.isOpen()) {
            return;
        }
    }
}

void AppManager::handleKey(sf::Keyboard::Key key) {
    switch (state_.screen()) {
    case Screen::MainMenu:
        // Mode and difficulty can only be chosen here, never during a game.
        switch (key) {
        case sf::Keyboard::Left:
        case sf::Keyboard::A:
            select(MenuInput::PreviousMode);
            break;
        case sf::Keyboard::Right:
        case sf::Keyboard::D:
            select(MenuInput::NextMode);
            break;
        case sf::Keyboard::Up:
        case sf::Keyboard::W:
            select(MenuInput::HarderDifficulty);
            break;
        case sf::Keyboard::Down:
        case sf::Keyboard::S:
            select(MenuInput::EasierDifficulty);
            break;
        case sf::Keyboard::Enter:
            startGame();
            break;
        case sf::Keyboard::Escape:
            dispatch(Action::Quit);
            break;
        default:
            break;
        }
        break;

    case Screen::Gameplay:
        // Movement controls: WASD or arrow keys.
        switch (key) {
        case sf::Keyboard::W:
        case sf::Keyboard::Up:
            gameplay_.changeDirection(Direction::Up);
            break;
        case sf::Keyboard::S:
        case sf::Keyboard::Down:
            gameplay_.changeDirection(Direction::Down);
            break;
        case sf::Keyboard::A:
        case sf::Keyboard::Left:
            gameplay_.changeDirection(Direction::Left);
            break;
        case sf::Keyboard::D:
        case sf::Keyboard::Right:
            gameplay_.changeDirection(Direction::Right);
            break;
        case sf::Keyboard::Enter:
            // Play again with the same settings once the game has stopped.
            if (gameplay_.isGameOver()) {
                startGame();
            }
            break;
        case sf::Keyboard::Escape:
            dispatch(Action::Pause);
            break;
        case sf::Keyboard::M:
            dispatch(Action::BackToMenu);
            break;
        default:
            break;
        }
        break;

    case Screen::Paused:
        switch (key) {
        case sf::Keyboard::Enter:
        case sf::Keyboard::Escape:
            dispatch(Action::Resume);
            break;
        case sf::Keyboard::M:
            dispatch(Action::BackToMenu);
            break;
        default:
            break;
        }
        break;

    case Screen::Exiting:
        break;
    }
}

void AppManager::select(MenuInput input) {
    selectedConfig_ = applyMenuInput(selectedConfig_, input, state_.screen());
    menuError_.clear();
}

void AppManager::startGame() {
    if (gameplay_.start(selectedConfig_)) {
        menuError_.clear();
        if (state_.screen() == Screen::MainMenu) {
            dispatch(Action::Start);
        }
        return;
    }

    // Stay on (or return to) the menu and say why.
    menuError_ = "Could not start the game: " + gameplay_.lastError();
    if (state_.screen() != Screen::MainMenu) {
        dispatch(Action::BackToMenu);
    }
}

void AppManager::dispatch(Action action) {
    state_.dispatch(action);

    if (state_.screen() == Screen::Exiting) {
        window_.close();
    } else {
        refreshTitle();
    }
}

void AppManager::update(sf::Time step) {
    // The controller ignores updates outside the Gameplay screen, so a
    // paused game keeps both its position and its countdown.
    gameplay_.update(std::chrono::microseconds{step.asMicroseconds()},
                     state_.screen());
}

void AppManager::refreshTitle() {
    // The title mirrors the HUD, so the state stays visible even if the
    // bundled font cannot be loaded.
    std::string title = titleText();
    if (title != title_) {
        title_ = std::move(title);
        window_.setTitle(title_);
    }
}

std::string AppManager::titleText() const {
    std::string title = "SnakeXtreme";

    switch (state_.screen()) {
    case Screen::MainMenu:
        title += " | Menu | " + settingsText(selectedConfig_);
        if (!menuError_.empty()) {
            title += " | " + menuError_;
        }
        title += " | Left/Right: mode, Up/Down: difficulty, Enter: start, Esc: exit";
        break;

    case Screen::Gameplay:
    case Screen::Paused: {
        if (gameplay_.isGameOver()) {
            title += " | Game over: " +
                     std::string(describeGameOver(gameplay_.gameOverReason()));
        } else if (state_.screen() == Screen::Paused) {
            title += " | Paused";
        } else {
            title += " | Playing";
        }
        title += " | " + settingsText(gameplay_.config()) +
                 " | Score " + std::to_string(gameplay_.score()) +
                 " | High " + std::to_string(gameplay_.highScore());
        if (const auto remaining = gameplay_.remainingTime()) {
            title += " | Time " + formatCountdown(*remaining);
        }
        if (state_.screen() == Screen::Paused) {
            title += " | Esc/Enter: resume, M: menu";
        } else if (gameplay_.isGameOver()) {
            title += " | Enter: play again, M: menu";
        } else {
            title += " | Esc: pause, M: menu";
        }
        break;
    }

    case Screen::Exiting:
        break;
    }

    return title;
}

FrameView AppManager::frame() const {
    FrameView view;
    view.screen = state_.screen();
    view.engine = &gameplay_.engine();
    view.selected = selectedConfig_;
    view.active = gameplay_.config();
    view.remainingTime = gameplay_.remainingTime();
    view.gameOverReason = gameplay_.gameOverReason();
    view.score = gameplay_.score();
    view.highScore = gameplay_.highScore();
    view.menuError = menuError_;
    return view;
}

} // namespace snake
