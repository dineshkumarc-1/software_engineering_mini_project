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

} // namespace snake::config
