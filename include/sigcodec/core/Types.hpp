#pragma once

#include <cstdint>
#include <vector>

namespace sigcodec {

// ============================================================
// Basic Types
// ============================================================

// One binary value: 0 or 1.
using Bit = std::uint8_t;

// Sequence of binary values.
using BitStream = std::vector<Bit>;

// Signal amplitude.
// Line coding normally uses -1, 0, +1.
// PCM/analog processing may use other values.
using Level = double;


// ============================================================
// Signal Representation
// ============================================================

// One point in the time-domain signal.
struct SignalPoint {
    double time{};
    Level level{};
};

// Complete time-domain signal.
using Signal = std::vector<SignalPoint>;

} // namespace sigcodec