#include "snakextreme/AppState.hpp"
#include <iostream>
#include <cstdlib>

namespace {
int checks = 0;
void expect(bool condition, const char* message) {
    ++checks;
    if (!condition) { std::cerr << "FAIL: " << message << '\n'; std::exit(1); }
}
}
int main() {
    using namespace snake;
    AppState state;
    expect(state.screen() == Screen::MainMenu, "launch enters menu");
    state.dispatch(Action::Resume);
    expect(state.screen() == Screen::MainMenu, "resume ignored in menu");
    state.dispatch(Action::Pause);
    expect(state.screen() == Screen::MainMenu, "pause ignored in menu");
    state.dispatch(Action::Start);
    expect(state.screen() == Screen::Gameplay, "start enters gameplay");
    state.dispatch(Action::Start);
    expect(state.screen() == Screen::Gameplay, "start ignored during gameplay");
    state.dispatch(Action::Pause);
    expect(state.screen() == Screen::Paused, "pause enters paused state");
    state.dispatch(Action::Start);
    expect(state.screen() == Screen::Paused, "start ignored while paused");
    state.dispatch(Action::Resume);
    expect(state.screen() == Screen::Gameplay, "resume restores gameplay");
    state.dispatch(Action::BackToMenu);
    expect(state.screen() == Screen::MainMenu, "gameplay returns to menu");
    state.dispatch(Action::Start); state.dispatch(Action::Pause);
    state.dispatch(Action::BackToMenu);
    expect(state.screen() == Screen::MainMenu, "pause returns to menu");
    for (Screen initial : {Screen::MainMenu, Screen::Gameplay, Screen::Paused}) {
        AppState app;
        if (initial != Screen::MainMenu) app.dispatch(Action::Start);
        if (initial == Screen::Paused) app.dispatch(Action::Pause);
        app.dispatch(Action::Quit);
        expect(app.screen() == Screen::Exiting, "quit works from every state");
        for (Action action : {Action::Start, Action::Pause, Action::Resume, Action::BackToMenu, Action::Quit}) {
            app.dispatch(action);
            expect(app.screen() == Screen::Exiting, "exiting is terminal");
        }
    }
    std::cout << checks << " state-flow checks passed\n";
}
