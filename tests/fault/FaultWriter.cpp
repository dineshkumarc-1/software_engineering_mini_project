// Child process for tests/SecureWriteFaultTests.cpp: performs exactly one
// save, so the LD_PRELOAD fault injector can crash or fail it.
//
//   secure_write_fault_writer leaderboard <path>   addScore("Crash", 400, Classic, Easy)
//   secure_write_fault_writer settings <path>      save Neon theme settings (if built)
//
// Exit code 0: saved; 3: the save reported an error; 4: score not qualified;
// 64/65: bad arguments. An injected crash ends the process with SIGABRT.

#include "snakextreme/GameMode.hpp"
#include "snakextreme/LeaderboardRepository.hpp"

#if defined(SNAKE_HAVE_SETTINGS)
#include "snakextreme/Settings.hpp"
#include "snakextreme/SettingsRepository.hpp"
#endif

#include <string>

int main(int argc, char** argv) {
    if (argc != 3) {
        return 64;
    }
    const std::string kind = argv[1];
    if (kind == "leaderboard") {
        const auto result = snake::LeaderboardRepository(argv[2]).addScore(
            "Crash", 400, snake::GameMode::Classic, snake::Difficulty::Easy);
        switch (result.status) {
        case snake::ScoreSubmissionStatus::Submitted:
            return 0;
        case snake::ScoreSubmissionStatus::NotQualified:
            return 4;
        case snake::ScoreSubmissionStatus::Error:
            return 3;
        }
        return 3;
    }
#if defined(SNAKE_HAVE_SETTINGS)
    if (kind == "settings") {
        snake::Settings settings;
        settings.theme = snake::Theme::Neon;
        settings.musicVolume = 15;
        std::string error;
        return snake::SettingsRepository(argv[2]).save(settings, error) ? 0 : 3;
    }
#endif
    return 65;
}
