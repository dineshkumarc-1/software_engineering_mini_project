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

} // namespace snake::config
