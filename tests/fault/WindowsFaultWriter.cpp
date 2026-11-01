// Child process for the native Windows TC-SEC-04 fault-injection tests.
// This executable links the real persistence implementation and explicitly
// loads the test-only IAT-hook DLL before performing exactly one save.

#include "snakextreme/GameMode.hpp"
#include "snakextreme/LeaderboardRepository.hpp"
#include "snakextreme/Settings.hpp"
#include "snakextreme/SettingsRepository.hpp"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>

#include <cstring>
#include <filesystem>
#include <string>

using InstallHooksFunction = BOOL(WINAPI*)(LPCWSTR, LPCWSTR);

int wmain(int argc, wchar_t* argv[]) {
    // kind, target path, fault mode, injector DLL path
    if (argc != 5) {
        return 64;
    }

    const std::wstring kind = argv[1];
    const std::filesystem::path targetPath(argv[2]);
    const std::wstring faultMode = argv[3];
    const std::filesystem::path injectorPath(argv[4]);

    HMODULE injector = ::LoadLibraryW(injectorPath.c_str());
    if (injector == nullptr) {
        return 65;
    }

    const FARPROC installAddress =
        ::GetProcAddress(injector, "SnakeInstallFaultHooks");
    InstallHooksFunction install{};
    static_assert(sizeof(install) == sizeof(installAddress),
        "Unexpected function pointer size");
    std::memcpy(&install, &installAddress, sizeof(install));
    if (install == nullptr || !install(faultMode.c_str(), targetPath.c_str())) {
        return 66;
    }

    if (kind == L"leaderboard") {
        const auto result = snake::LeaderboardRepository(targetPath).addScore(
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

    if (kind == L"settings") {
        snake::Settings settings;
        settings.theme = snake::Theme::Neon;
        settings.musicVolume = 15;
        std::string error;
        return snake::SettingsRepository(targetPath).save(settings, error) ? 0 : 3;
    }

    return 67;
}
