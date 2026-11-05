# Branch coverage report — Part 3

**Tested source:** `04d782a345464349da10c91d6796b3ed99fc7b96` (tree `e7f5e8ef5f943d067375af0b375420bf81b421f3`), clean detached checkout of main.  
**Run:** 5 November 2026 UTC, automated Linux headless Debug coverage build; not a Windows or human acceptance test.  
**Status:** In progress; no final QA sign-off.

Same instrumented headless run as the [line report](CODE_COVERAGE_REPORT.md). **1,706 / 2,600 compiler-reported branches taken = 65.62%** across measured files. GCC branches include compiler-generated control flow such as exception and short-circuit paths; these are not one-to-one with requirements or test cases. No project-wide branch threshold was specified in the submitted STP. Desktop/presentation and platform-specific code not built by the headless configuration are excluded.

| Measured file | Branches taken / total | Coverage |
|---|---:|---:|
| `include/snakextreme/DifficultyProfile.hpp` | 3/4 | 75.00% |
| `include/snakextreme/GameEngine.hpp` | 4/4 | 100.00% |
| `include/snakextreme/GameMode.hpp` | 14/16 | 87.50% |
| `include/snakextreme/GameSession.hpp` | 2/2 | 100.00% |
| `include/snakextreme/LeaderboardModel.hpp` | 1/2 | 50.00% |
| `include/snakextreme/LeaderboardTable.hpp` | 15/30 | 50.00% |
| `include/snakextreme/MenuSelection.hpp` | 15/17 | 88.24% |
| `include/snakextreme/NameEntry.hpp` | 26/36 | 72.22% |
| `include/snakextreme/PowerUp.hpp` | 3/4 | 75.00% |
| `include/snakextreme/UiHitTest.hpp` | 41/54 | 75.93% |
| `src/app/AppState.cpp` | 35/39 | 89.74% |
| `src/audio/AudioModel.cpp` | 25/32 | 78.12% |
| `src/diagnostics/PerformanceProbe.cpp` | 242/462 | 52.38% |
| `src/engine/GameEngine.cpp` | 159/217 | 73.27% |
| `src/gameplay/GameplayController.cpp` | 106/141 | 75.18% |
| `src/modes/ModeManager.cpp` | 19/24 | 79.17% |
| `src/modes/ModeStrategy.cpp` | 90/110 | 81.82% |
| `src/persistence/LeaderboardModel.cpp` | 10/32 | 31.25% |
| `src/persistence/LeaderboardRepository.cpp` | 245/364 | 67.31% |
| `src/persistence/SecureFileStore.cpp` | 306/504 | 60.71% |
| `src/persistence/Settings.cpp` | 80/117 | 68.38% |
| `src/persistence/SettingsRepository.cpp` | 117/182 | 64.29% |
| `src/powerups/PowerUpEffects.cpp` | 26/44 | 59.09% |
| `src/powerups/PowerUpManager.cpp` | 79/110 | 71.82% |
| `src/scoring/ScoreService.cpp` | 4/5 | 80.00% |
| `src/validation/InputValidator.cpp` | 39/48 | 81.25% |

Files with zero reported branches are omitted from this table. See [machine-readable summary and CTest output](EVIDENCE.md). A passing branch metric does not establish final validation or universal correctness.
