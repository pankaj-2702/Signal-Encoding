#pragma once

#include "sigcodec/core/Types.hpp"

namespace sigcodec {

// ============================================================
// Standard Signal Levels
// ============================================================

namespace levels {

constexpr Level Positive = 1.0;
constexpr Level Zero     = 0.0;
constexpr Level Negative = -1.0;

} // namespace levels


// ============================================================
// Initial Polarity
// ============================================================

enum class InitialPolarity {
    Positive,
    Negative
};


// ============================================================
// Transition
// ============================================================

enum class Transition {
    NoTransition,
    Transition
};

} // namespace sigcodec