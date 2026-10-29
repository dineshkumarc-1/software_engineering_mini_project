#pragma once

#include <cstddef>
#include <cstdint>

namespace snake::config {

inline constexpr int GridColumns = 30;
inline constexpr int GridRows = 20;
inline constexpr int CellSize = 30;

inline constexpr int WindowWidth = GridColumns * CellSize;
inline constexpr int WindowHeight = GridRows * CellSize;

inline constexpr std::size_t InitialSnakeLength = 3;

inline constexpr std::uint32_t EasyMoveIntervalMs = 200;
inline constexpr std::uint32_t MediumMoveIntervalMs = 120;
inline constexpr std::uint32_t HardMoveIntervalMs = 70;

// Game-mode baselines (SRS Addendum v1.1 section 4, issues #11 and #12).
inline constexpr std::uint32_t TimedChallengeDurationMs = 60000;
inline constexpr std::size_t ObstacleCount = 8;
// Obstacles keep at least this Chebyshev distance from the snake head, so the
// 5x5 square centred on the head stays clear at session start.
inline constexpr int ObstacleMinHeadDistance = 3;

// Points per food before any power-up multiplier (FR-016).
inline constexpr int ClassicFoodPoints = 10;
inline constexpr int TimedFoodPoints = 10;
inline constexpr int ObstacleFoodPoints = 15;

// Power-ups (SRS Addendum v1.1 section 4, FR-012..015, issue #14).
// A new power-up appears 8-15 s after the previous one (FR-012: never more
// than 15 s apart) and disappears after 5 s if it is not collected.
inline constexpr std::uint32_t PowerUpSpawnGapMinMs = 8000;
inline constexpr std::uint32_t PowerUpSpawnGapMaxMs = 15000;
inline constexpr std::uint32_t PowerUpLifetimeMs = 5000;
// Speed: tick interval x 0.6 for 5 s (FR-013), kept as a fraction so
// 200 / 120 / 70 ms become exactly 120 / 72 / 42 ms.
inline constexpr std::uint32_t SpeedBoostDurationMs = 5000;
inline constexpr int SpeedBoostNumerator = 3;
inline constexpr int SpeedBoostDenominator = 5;
// Multiplier: food points x 2 for 10 s (FR-014).
inline constexpr std::uint32_t MultiplierDurationMs = 10000;
inline constexpr int ScoreMultiplier = 2;
// Shrink: one tail segment, never below 3 segments (FR-015, SRS 6.5).
inline constexpr std::size_t ShrinkSegments = 1;
inline constexpr std::size_t MinimumSnakeLength = 3;

} // namespace snake::config
