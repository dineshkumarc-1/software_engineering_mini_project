# Code coverage report — Part 3

**Tested source:** `04d782a345464349da10c91d6796b3ed99fc7b96` (tree `e7f5e8ef5f943d067375af0b375420bf81b421f3`), clean detached checkout of main.  
**Run:** 5 November 2026 UTC, automated Linux headless Debug coverage build; not a Windows or human acceptance test.  
**Status:** In progress; no final QA sign-off.

GCC `--coverage -O0`, gcovr 8.8 after the 21-entry CTest run. Filters include only instrumented project `src/` and `include/` files found in the headless build. **1,686 / 1,798 executable lines = 93.77%** across those measured files, not whole-product coverage. Desktop/SFML presentation code and platform-specific code not built by this configuration are excluded. Executable header lines observed through compiled tests are included.

## STP C6–C11 component criterion

| Component and mapped measured files | Covered / executable lines | Line coverage |
|---|---:|---:|
| C6 (src/engine/GameEngine.cpp, include/snakextreme/GameEngine.hpp) | 209/213 | 98.12% |
| C7 (src/modes/ModeManager.cpp, src/modes/ModeStrategy.cpp, include/snakextreme/GameMode.hpp, include/snakextreme/ModeManager.hpp, include/snakextreme/ModeStrategy.hpp, include/snakextreme/DifficultyProfile.hpp) | 205/212 | 96.70% |
| C8 (src/powerups/PowerUpManager.cpp, src/powerups/PowerUpEffects.cpp, include/snakextreme/PowerUp.hpp, include/snakextreme/PowerUpManager.hpp) | 178/183 | 97.27% |
| C9 (src/scoring/ScoreService.cpp) | 24/25 | 96.00% |
| C10 (src/validation/InputValidator.cpp) | 36/37 | 97.30% |
| C11 (src/persistence/LeaderboardRepository.cpp) | 180/196 | 91.84% |

Each mapped component meets the STP 80% line threshold in this Linux build. This does not imply every file reaches 80%, or that system validation is complete. Mapping follows the previous [component report](../qa/OWNER-AUTOMATED-QA-7c6733d.md).

[Per-file line output, machine-readable summary and CTest output](EVIDENCE.md)

Collection used gcovr project-relative `src/` and `include/` filters, excluding CMake compiler-ID probes and dependency build directories. Initial collection attempts failed on compiler-ID source lookup and an overly broad filter; the final scoped collection succeeded without changing source, assertions or counters.
