
#include "snakextreme/AppManager.hpp"
#include "snakextreme/GameConfig.hpp"

#include <algorithm>

namespace snake {

AppManager::AppManager()
    : window_(sf::VideoMode(Renderer::Width, Renderer::Height),
              "SnakeXtreme",
              sf::Style::Titlebar | sf::Style::Close) {
    window_.setFramerateLimit(60);
    window_.setKeyRepeatEnabled(false);
    updateTitle();
}

int AppManager::run() {
    sf::Clock clock;
    sf::Time accumulator = sf::Time::Zero;
    const auto fixedStep = sf::seconds(1.f / 60.f);

    while (window_.isOpen()) {
        const auto elapsed = clock.restart();
        pollEvents();

        if (!window_.isOpen()) {
            break;
        }

        accumulator += std::min(elapsed, sf::milliseconds(250));

        while (accumulator >= fixedStep) {
            update(fixedStep);
            accumulator -= fixedStep;
        }

        window_.clear(sf::Color(15, 23, 32));
        renderer_.draw(window_, state_.screen(), engine_);
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

        if (event.type != sf::Event::KeyPressed) {
            continue;
        }

        // Movement input is accepted only during gameplay.
        if (state_.screen() == Screen::Gameplay) {
            switch (event.key.code) {
            case sf::Keyboard::W:
            case sf::Keyboard::Up:
                engine_.changeDirection(Direction::Up);
                break;

            case sf::Keyboard::S:
            case sf::Keyboard::Down:
                engine_.changeDirection(Direction::Down);
                break;

            case sf::Keyboard::A:
            case sf::Keyboard::Left:
                engine_.changeDirection(Direction::Left);
                break;

            case sf::Keyboard::D:
            case sf::Keyboard::Right:
                engine_.changeDirection(Direction::Right);
                break;

            default:
                break;
            }
        }

        switch (event.key.code) {
        case sf::Keyboard::Enter:
            if (state_.screen() == Screen::MainMenu) {
                dispatch(Action::Start);
            } else if (state_.screen() == Screen::Paused) {
                dispatch(Action::Resume);
            }
            break;

        case sf::Keyboard::Escape:
            if (state_.screen() == Screen::MainMenu) {
                dispatch(Action::Quit);
            } else if (state_.screen() == Screen::Gameplay) {
                dispatch(Action::Pause);
            } else if (state_.screen() == Screen::Paused) {
                dispatch(Action::Resume);
            }
            break;

        case sf::Keyboard::M:
            dispatch(Action::BackToMenu);
            break;

        default:
            break;
        }

        if (!window_.isOpen()) {
            return;
        }
    }
}

void AppManager::dispatch(Action action) {
    state_.dispatch(action);

    if (action == Action::Start) {
        engine_ = GameEngine{};
        movementAccumulator_ = sf::Time::Zero;
    }

    if (action == Action::Resume) {
        movementAccumulator_ = sf::Time::Zero;
    }

    if (state_.screen() == Screen::Exiting) {
        window_.close();
    } else {
        updateTitle();
    }
}

void AppManager::update(sf::Time step) {
    if (state_.screen() != Screen::Gameplay || engine_.isGameOver()) {
        return;
    }

    movementAccumulator_ += step;

    const auto moveInterval =
        sf::milliseconds(config::EasyMoveIntervalMs);

    while (movementAccumulator_ >= moveInterval &&
           !engine_.isGameOver()) {
        engine_.tick();
        movementAccumulator_ -= moveInterval;
    }
}

void AppManager::updateTitle() {
    const char* title = "SnakeXtreme";

    switch (state_.screen()) {
    case Screen::MainMenu:
        title = "SnakeXtreme | Main menu | Enter: start | Esc: exit";
        break;

    case Screen::Gameplay:
        title = "SnakeXtreme | WASD/Arrows: move | Esc: pause | M: menu";
        break;

    case Screen::Paused:
        title = "SnakeXtreme | Paused | Esc/Enter: resume | M: menu";
        break;

    case Screen::Exiting:
        break;
    }

    window_.setTitle(title);
}

} // namespace snake
