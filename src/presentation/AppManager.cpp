#include "snakextreme/AppManager.hpp"
#include <algorithm>

namespace snake {
AppManager::AppManager()
    : window_(sf::VideoMode(Renderer::Width, Renderer::Height), "SnakeXtreme",
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
        // Never update, clear, or display after the close event.
        if (!window_.isOpen()) break;
        // Limit catch-up after a debugger break or OS stall.
        accumulator += std::min(elapsed, sf::milliseconds(250));
        while (accumulator >= fixedStep) {
            update(fixedStep);
            accumulator -= fixedStep;
        }
        window_.clear(sf::Color(15, 23, 32));
        renderer_.draw(
            window_,
            state_.screen(),
            gameSession_.currentScore(),
            gameSession_.highScore()
        );
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
        if (event.type == sf::Event::LostFocus && state_.screen() == Screen::Gameplay)
            dispatch(Action::Pause);
        if (event.type != sf::Event::KeyPressed) continue;
        switch (event.key.code) {
        case sf::Keyboard::Enter:
            if (state_.screen() == Screen::MainMenu) dispatch(Action::Start);
            else if (state_.screen() == Screen::Paused) dispatch(Action::Resume);
            break;
        case sf::Keyboard::Escape:
            if (state_.screen() == Screen::MainMenu) dispatch(Action::Quit);
            else if (state_.screen() == Screen::Gameplay) dispatch(Action::Pause);
            else if (state_.screen() == Screen::Paused) dispatch(Action::Resume);
            break;
        case sf::Keyboard::M:
            dispatch(Action::BackToMenu);
            break;
        default: break;
        }
        if (!window_.isOpen()) return;
    }
}

void AppManager::dispatch(Action action) {
    if (action == Action::Start) {
        gameSession_.start();
    }

    state_.dispatch(action);

    if (state_.screen() == Screen::Exiting) {
        window_.close();
    } else {
        updateTitle();
    }
}
void AppManager::update(sf::Time step) {
    if (state_.screen() != Screen::Gameplay) return;
    // Integration seam for the SFML-free engine; do not put game rules here.
    // Engine tick intervals (200/120/70 ms) remain the engine owner's concern.
    (void)step;
}

void AppManager::updateTitle() {
    // Font-free placeholder UI: useful controls are visible in the title bar.
    // Full text/menu screens belong to later FR-022..029 stories.
    const char* title = "SnakeXtreme";
    switch (state_.screen()) {
    case Screen::MainMenu:
        title = "SnakeXtreme | Main menu scaffold | Enter: preview | Esc: exit"; break;
    case Screen::Gameplay:
        title = "SnakeXtreme | Gameplay placeholder | Esc: pause | M: menu"; break;
    case Screen::Paused:
        title = "SnakeXtreme | Paused | Esc/Enter: resume | M: menu"; break;
    case Screen::Exiting: break;
    }
    window_.setTitle(title);
}
} // namespace snake
