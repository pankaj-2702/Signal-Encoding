
#pragma once

#include <cstdint>
#include <vector>

namespace sigcodec {

using Bit = std::uint8_t;
using BitStream = std::vector<Bit>;
using Level = double;

struct SignalPoint {
    double time{};
    Level level{};
};

using Signal = std::vector<SignalPoint>;

// One signed pulse level per input bit.
// +1 = positive pulse, 0 = no pulse, -1 = negative pulse.
using BipolarSignal = std::vector<Level>;

} // namespace sigcodec
