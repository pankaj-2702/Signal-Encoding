
#include "sigcodec/linecode/HDB3.hpp"

#include <stdexcept>

namespace sigcodec {

BipolarSignal HDB3::encode(const BitStream& bits) const {
    BipolarSignal output;
    output.reserve(bits.size());

    // The first ordinary AMI mark will be positive.
    Level lastPulse = -1.0;

    // Count ordinary nonzero pulses since the last substitution.
    std::size_t pulseCount = 0;

    for (std::size_t i = 0; i < bits.size();) {
        if (bits[i] > 1) {
            throw std::invalid_argument("Input must contain only 0 and 1.");
        }

        // Find a run of four consecutive zeros.
        bool fourZeros = i + 4 <= bits.size();

        if (fourZeros) {
            for (std::size_t j = 0; j < 4; ++j) {
                if (bits[i + j] != 0) {
                    fourZeros = false;
                    break;
                }
            }
        }

       
if (fourZeros) {
    if (pulseCount % 2 == 1) {
        // Odd number of ordinary pulses: 000V.
        // V has the same polarity as the previous pulse.
        const Level V = lastPulse;

        output.insert(output.end(), {0.0, 0.0, 0.0, V});
        lastPulse = V;
    } else {
        // Even number of ordinary pulses: B00V.
        const Level B = -lastPulse;
        const Level V = B;

        output.insert(output.end(), {B, 0.0, 0.0, V});
        lastPulse = V;
    }

    pulseCount = 0;
    i += 4;
    continue;
}


        if (bits[i] == 0) {
            output.push_back(0.0);
        } else {
            lastPulse = -lastPulse;
            output.push_back(lastPulse);
            ++pulseCount;
        }

        ++i;
    }

    return output;
}

std::string HDB3::name() const {
    return "HDB3";
}

} // namespace sigcodec
