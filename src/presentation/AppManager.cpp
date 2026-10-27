
#include "snakextreme/AppManager.hpp"
#include "snakextreme/UiHitTest.hpp"

#include <algorithm>
#include <chrono>
#include <iostream>

namespace snake {
static_assert(sf::Keyboard::Up == 73 && sf::Keyboard::Down == 74 &&
              sf::Keyboard::Left == 71 && sf::Keyboard::Right == 72 &&
              sf::Keyboard::W == 22 && sf::Keyboard::S == 18 &&
              sf::Keyboard::A == 0 && sf::Keyboard::D == 3,
              "Settings schema requires SFML 2.x key codes");

namespace {

std::string settingsText(const SessionConfig& config) {
    return std::string(toString(config.mode)) + " / " +
           std::string(toString(config.difficulty));
}

} // namespace

AppManager::AppManager(PerformanceProbe* probe, std::chrono::steady_clock::time_point mainEntry,
                       std::optional<int> probeSeconds)
    : probe_(probe), probeStart_(mainEntry), probeSeconds_(probeSeconds), window_(
          sf::VideoMode(Renderer::Width, Renderer::Height),
          "SnakeXtreme",
          sf::Style::Titlebar | sf::Style::Close) {
    window_.setFramerateLimit(60);
    window_.setKeyRepeatEnabled(false);
    const auto loaded = settingsRepository_.load();
    settings_ = loaded.settings;
    if (loaded.usedDefaults) {
        settingsNotice_ = loaded.warning;
        std::cerr << "[WARN] SettingsRepository: " << loaded.warning << '\n';
        std::string error;
        if (!settingsRepository_.save(settings_, error)) {
            settingsNotice_ += "; " + error;
            std::cerr << "[WARN] SettingsRepository: " << error << '\n';
        }
    }
    settingsEditor_.open(settings_);
    if (!audio_.start(settings_.musicVolume, settings_.sfxVolume))
        std::cerr << "[WARN] AudioService: playback unavailable; silent fallback\n";
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

        window_.clear(Renderer::background(state_.screen() == Screen::Settings
            ? settingsEditor_.value().theme : settings_.theme));
        renderer_.draw(window_, frame());
        window_.display();
        if (probe_ != nullptr) {
            const auto now = probeNow();
            const bool first = !probe_->firstFrameUs();
            probe_->frame(now, probeContext());
            if (first) std::cout << "SNAKEXTREME_PROBE_FIRST_FRAME\n" << std::flush;
            if (probeSeconds_ && probe_->firstFrameUs() &&
                now - *probe_->firstFrameUs() >= static_cast<std::int64_t>(*probeSeconds_) * 1000000)
                dispatch(Action::Quit);
        }
    }

    return 0;
}

std::int64_t AppManager::probeNow() const {
    return std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::steady_clock::now() - probeStart_).count();
}
ProbeContext AppManager::probeContext() const {
    const char* screen = "Other";
    switch (state_.screen()) {
    case Screen::MainMenu: screen = "MainMenu"; break;
    case Screen::NameEntry: screen = "NameEntry"; break;
    case Screen::ModeSelection: screen = "ModeSelection"; break;
    case Screen::Gameplay: screen = "Gameplay"; break;
    case Screen::Paused: screen = "Paused"; break;
    case Screen::GameOver: screen = "GameOver"; break;
    case Screen::Leaderboard: screen = "Leaderboard"; break;
    case Screen::Settings: screen = "Settings"; break;
    case Screen::Exiting: screen = "Exiting"; break;
    }
    return {screen, std::string(toString(gameplay_.config().mode)),
                    std::string(toString(gameplay_.config().difficulty))};
}

void AppManager::pollEvents() {
    sf::Event event{};

    while (window_.pollEvent(event)) {
        if (probe_ != nullptr) inputDequeuedUs_ = probeNow();
        const int oldSfxVolume = state_.screen() == Screen::Settings
            ? settingsEditor_.value().sfxVolume : settings_.sfxVolume;
        if (event.type == sf::Event::Closed) {
            dispatch(Action::Quit);
            return;
        }

        if (event.type == sf::Event::LostFocus &&
            state_.screen() == Screen::Gameplay) {
            dispatch(Action::Pause);
        }

        if (event.type == sf::Event::TextEntered && state_.screen() == Screen::NameEntry) {
            nameEntry_.input(event.text.unicode);
        }
        if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
            if (state_.screen() == Screen::Settings) clickSettings(event.mouseButton.x, event.mouseButton.y);
            else clickNavigation(event.mouseButton.x, event.mouseButton.y);
        }
        if (event.type == sf::Event::MouseButtonReleased || event.type == sf::Event::LostFocus)
            draggingVolume_ = false;
        if (event.type == sf::Event::MouseMoved && state_.screen() == Screen::Settings &&
            draggingVolume_ && !settingsEditor_.capturing()) {
            // Continue a slider drag even when the pointer leaves its track.
            if (settingsEditor_.row() == 4 || settingsEditor_.row() == 5)
                settingsEditor_.setVolume((event.mouseMove.x - 400) / 3);
        }
        if (event.type == sf::Event::KeyPressed) {
            handleKey(event.key.code);
        }

        if (!window_.isOpen()) {
            return;
        }
        const int currentSfxVolume = state_.screen() == Screen::Settings
            ? settingsEditor_.value().sfxVolume : settings_.sfxVolume;
        syncAudio(state_.screen() == Screen::Settings && currentSfxVolume != oldSfxVolume);
    }
}

void AppManager::handleKey(sf::Keyboard::Key key) {
    switch (state_.screen()) {
    case Screen::MainMenu:
        if (key == sf::Keyboard::Up || key == sf::Keyboard::W) menuIndex_ = (menuIndex_ + 3) % 4;
        if (key == sf::Keyboard::Down || key == sf::Keyboard::S) menuIndex_ = (menuIndex_ + 1) % 4;
        if (key == sf::Keyboard::Escape) dispatch(Action::Quit);
        if (key == sf::Keyboard::Enter) {
            const Action actions[] = {Action::Start, Action::OpenLeaderboard, Action::OpenSettings, Action::Quit};
            if (menuIndex_ == 0) nameEntry_ = NameEntry{};
            dispatch(actions[menuIndex_]);
        }
        break;
    case Screen::NameEntry:
        if (key == sf::Keyboard::Escape) dispatch(Action::Back);
        if (key == sf::Keyboard::Enter && nameEntry_.confirm()) dispatch(Action::ConfirmName);
        break;
    case Screen::Settings:
        handleSettingsKey(key);
        break;
    case Screen::Leaderboard:
        if (key == sf::Keyboard::Escape) dispatch(Action::Back);
        break;
    case Screen::GameOver:
        if (key == sf::Keyboard::Enter) startGame();
        if (key == sf::Keyboard::M || key == sf::Keyboard::Escape) dispatch(Action::BackToMenu);
        break;
    case Screen::ModeSelection:
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
            dispatch(Action::Back);
            break;
        default:
            break;
        }
        break;

    case Screen::Gameplay:
        // Primary bindings plus permanent WASD aliases, independent of SFML in the model.
        if (const auto movement = movementForKey(settings_, static_cast<int>(key))) {
            constexpr Direction directions[] = {Direction::Up, Direction::Down, Direction::Left, Direction::Right};
            const bool accepted = gameplay_.requestDirection(directions[*movement]);
            if (probe_ != nullptr) {
                constexpr const char* names[] = {"Up", "Down", "Left", "Right"};
                probe_->input(inputDequeuedUs_, probeNow(), accepted, names[*movement]);
            }
            break;
        }
        switch (key) {
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

void AppManager::syncAudio(bool previewSfx) {
    const auto& volumes = state_.screen() == Screen::Settings ? settingsEditor_.value() : settings_;
    audio_.setVolumes(volumes.musicVolume, volumes.sfxVolume);
    audio_.setPaused(state_.screen() == Screen::Paused || state_.screen() == Screen::Exiting);
    if (previewSfx) audio_.playSfx(SoundCue::Preview);
}

void AppManager::handleSettingsKey(sf::Keyboard::Key key) {
    if (settingsEditor_.capturing()) {
        if (key == sf::Keyboard::Escape) settingsEditor_.cancelCapture();
        else settingsEditor_.bind(static_cast<int>(key));
        return;
    }
    if (key == sf::Keyboard::F5) { audio_.playSfx(SoundCue::Preview); return; }
    switch (key) {
    case sf::Keyboard::Up: settingsEditor_.move(-1); break;
    case sf::Keyboard::Down: settingsEditor_.move(1); break;
    case sf::Keyboard::Left: settingsEditor_.adjust(settingsEditor_.row() == 6 ? -1 : -5); break;
    case sf::Keyboard::Right: settingsEditor_.adjust(settingsEditor_.row() == 6 ? 1 : 5); break;
    case sf::Keyboard::Enter: activateSettings(); break;
    case sf::Keyboard::Escape: dispatch(Action::Back); break;
    default: break;
    }
}
void AppManager::activateSettings() {
    if (settingsEditor_.row() == 8) {
        std::string error;
        if (!settingsRepository_.save(settingsEditor_.value(), error)) {
            settingsEditor_.error(error); return;
        }
        settings_ = settingsEditor_.value();
        settingsNotice_.clear();
        dispatch(Action::Back);
    } else if (settingsEditor_.row() == 9) dispatch(Action::Back);
    else settingsEditor_.activate();
}
void AppManager::clickSettings(int x, int y) {
    draggingVolume_ = false;
    const auto row = settingsRowAt(x, y);
    if (settingsEditor_.capturing() || !row) return;
    settingsEditor_.selectRow(*row);
    if ((settingsEditor_.row() == 4 || settingsEditor_.row() == 5) && x >= 400 && x <= 700) {
        draggingVolume_ = true;
        settingsEditor_.setVolume((x - 400) / 3);
    } else activateSettings();
}

void AppManager::clickNavigation(int x, int y) {
    const auto hit = navigationHit(state_.screen(), x, y);
    if (!hit) return;
    switch (hit->command) {
    case UiCommand::MainItem: menuIndex_ = hit->index; handleKey(sf::Keyboard::Enter); break;
    case UiCommand::Mode: selectedConfig_.mode = AllGameModes[hit->index]; menuError_.clear(); break;
    case UiCommand::Difficulty: selectedConfig_.difficulty = AllDifficulties[hit->index]; menuError_.clear(); break;
    case UiCommand::Confirm: handleKey(sf::Keyboard::Enter); break;
    case UiCommand::Back: dispatch(Action::Back); break;
    case UiCommand::Resume: dispatch(Action::Resume); break;
    case UiCommand::Restart: startGame(); break;
    case UiCommand::Menu: dispatch(Action::BackToMenu); break;
    }
}

void AppManager::select(MenuInput input) {
    selectedConfig_ = applyMenuInput(selectedConfig_, input, state_.screen());
    menuError_.clear();
}

void AppManager::startGame() {
    if (gameplay_.start(selectedConfig_)) {
        if (probe_ != nullptr) probe_->beginSession();
        menuError_.clear();
        if (state_.screen() == Screen::ModeSelection) dispatch(Action::BeginGame);
        else if (state_.screen() == Screen::GameOver) dispatch(Action::Restart);
        return;
    }

    // Stay on (or return to) the menu and say why.
    menuError_ = "Could not start the game: " + gameplay_.lastError();
    if (state_.screen() != Screen::MainMenu) {
        dispatch(Action::BackToMenu);
    }
}

void AppManager::dispatch(Action action) {
    draggingVolume_ = false;
    const Screen previous = state_.screen();
    state_.dispatch(action);
    if (state_.screen() == Screen::Settings && previous != Screen::Settings) {
        settingsEditor_.open(settings_);
        settingsEditor_.error(settingsNotice_);
    }

    syncAudio();
    if (state_.screen() == Screen::Exiting) {
        window_.close();
    } else {
        refreshTitle();
    }
}

void AppManager::update(sf::Time step) {
    // The controller ignores updates outside the Gameplay screen, so a
    // paused game keeps both its position and its countdown.
    const int previousScore = gameplay_.score();
    gameplay_.update(std::chrono::microseconds{step.asMicroseconds()},
                     state_.screen());
    if (probe_ != nullptr) probe_->engineApplied(gameplay_.engine().appliedDirectionCount(), probeNow());
    if (state_.screen() == Screen::Gameplay && gameplay_.score() > previousScore)
        audio_.playSfx(SoundCue::Food);
    if (state_.screen() == Screen::Gameplay && gameplay_.isGameOver()) {
        dispatch(Action::EndGame);
        audio_.playSfx(SoundCue::GameOver);
    }
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
        title += " | Main menu | Up/Down: select, Enter: confirm, Esc: exit";
        break;
    case Screen::NameEntry:
        title += " | Name: " + nameEntry_.value() + " | " + nameEntry_.error();
        break;
    case Screen::Leaderboard:
        title += " | Leaderboard integration pending (#21) | Esc: back";
        break;
    case Screen::Settings:
        title += " | Settings | row " + std::to_string(settingsEditor_.row() + 1) +
            " | Music " + std::to_string(settingsEditor_.value().musicVolume) +
            " SFX " + std::to_string(settingsEditor_.value().sfxVolume) +
            " | " + std::string(themeName(settingsEditor_.value().theme)) +
            " | " + settingsEditor_.message() + " | Enter: edit/save, Esc: cancel";
        break;
    case Screen::ModeSelection:
        title +=  " | Menu | " + settingsText(selectedConfig_);
        if (!menuError_.empty()) {
            title += " | " + menuError_;
        }
        title += " | Left/Right: mode, Up/Down: difficulty, Enter: start, Esc: exit";
        break;

    case Screen::Gameplay:
    case Screen::GameOver:
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
    view.audioAvailable = audio_.available();
    view.settings = state_.screen() == Screen::Settings ? settingsEditor_.value() : settings_;
    view.settingsRow = settingsEditor_.row();
    view.capturingKey = settingsEditor_.capturing();
    view.settingsMessage = settingsEditor_.message();
    view.screen = state_.screen();
    view.engine = &gameplay_.engine();
    view.selected = selectedConfig_;
    view.active = gameplay_.config();
    view.remainingTime = gameplay_.remainingTime();
    view.gameOverReason = gameplay_.gameOverReason();
    view.score = gameplay_.score();
    view.highScore = gameplay_.highScore();
    view.menuError = menuError_;
    view.playerName = nameEntry_.value();
    view.nameError = nameEntry_.error();
    view.menuIndex = menuIndex_;
    return view;
}

} // namespace snake
