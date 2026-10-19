
#pragma once

#include <cstddef>
#include <vector>
#include <string>
#include "sigcodec/core/Types.hpp"

namespace sigcodec {

struct PcmResult {
    BitStream bits;
    std::vector<double> reconstructedSamples;
    Signal staircase;
};

class Pcm {
public:
    // Convert analog samples into a binary bitstream.
    BitStream encode(
        const std::vector<double>& samples,
        int bitsPerSample = 8
    ) const;

    // Convert the PCM bitstream back into quantized samples.
    PcmResult decode(
        const BitStream& bits,
        int bitsPerSample = 8
    ) const;

    // Convert a bitstream to its encoded bit string for display.
    static std::string bitsToString(const BitStream& bits);
};

}
