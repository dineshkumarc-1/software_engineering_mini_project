
#include "snakextreme/AppManager.hpp"

#include <algorithm>

namespace snake {

AppManager::AppManager()
    : window_(
          sf::VideoMode(Renderer::Width, Renderer::Height),
          "SnakeXtreme",
          sf::Style::Titlebar | sf::Style::Close) {
    window_.setFramerateLimit(60);
    window_.setKeyRepeatEnabled(false);
    updateTitle();
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

        window_.clear(sf::Color(15, 23, 32));

        renderer_.draw(
            window_,
            state_.screen(),
            gameEngine_,
            gameSession_.currentScore(),
            gameSession_.highScore());

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

        // Movement controls: WASD or arrow keys.
        if (state_.screen() == Screen::Gameplay) {
            switch (event.key.code) {
            case sf::Keyboard::W:
            case sf::Keyboard::Up:
                gameEngine_.changeDirection(Direction::Up);
                break;

            case sf::Keyboard::S:
            case sf::Keyboard::Down:
                gameEngine_.changeDirection(Direction::Down);
                break;

            case sf::Keyboard::A:
            case sf::Keyboard::Left:
                gameEngine_.changeDirection(Direction::Left);
                break;

            case sf::Keyboard::D:
            case sf::Keyboard::Right:
                gameEngine_.changeDirection(Direction::Right);
                break;

            default:
                break;
            }
        }

        // Menu and gameplay actions.
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
    if (action == Action::Start) {
        gameEngine_ = GameEngine{};
        gameSession_.start();
        movementAccumulator_ = sf::Time::Zero;
    }

    state_.dispatch(action);

    if (state_.screen() == Screen::Exiting) {
        window_.close();
    } else {
        updateTitle();
    }
}

void AppManager::update(sf::Time step) {
    if (state_.screen() != Screen::Gameplay ||
        gameEngine_.isGameOver()) {
        return;
    }

    // Keep movement timing independent of the rendering frame rate.
    constexpr float movementIntervalSeconds = 0.120f;
    const sf::Time movementInterval =
        sf::seconds(movementIntervalSeconds);

    movementAccumulator_ += step;

    while (movementAccumulator_ >= movementInterval &&
           !gameEngine_.isGameOver()) {
        gameSession_.tick(gameEngine_, activeMode_);
        movementAccumulator_ -= movementInterval;
    }
}

void AppManager::updateTitle() {
    // Keep the controls visible until the full menu UI is available.
    const char* title = "SnakeXtreme";

    switch (state_.screen()) {
    case Screen::MainMenu:
        title =
            "SnakeXtreme | Main menu scaffold | Enter: preview | Esc: exit";
        break;

    case Screen::Gameplay:
        title =
            "SnakeXtreme | Gameplay placeholder | Esc: pause | M: menu";
        break;

    case Screen::Paused:
        title =
            "SnakeXtreme | Paused | Esc/Enter: resume | M: menu";
        break;

    case Screen::Exiting:
        break;
    }

    window_.setTitle(title);
}

} // namespace snake
