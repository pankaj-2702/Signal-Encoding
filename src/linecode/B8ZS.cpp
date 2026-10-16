
#include "sigcodec/linecode/B8ZS.hpp"

#include <stdexcept>

namespace sigcodec {

BipolarSignal B8ZS::encode(const BitStream& bits) const {
    BipolarSignal output;
    output.reserve(bits.size());

    // Startup convention: the initial reference polarity is negative.
    // Therefore, the first ordinary AMI pulse is positive.
    Level lastPulse = -1.0;

    for (std::size_t i = 0; i < bits.size();) {
        if (bits[i] > 1) {
            throw std::invalid_argument(
                "Input must contain only 0 and 1."
            );
        }

        // Check whether eight consecutive zeros begin at position i.
        bool eightZeros = i + 8 <= bits.size();

        if (eightZeros) {
            for (std::size_t j = 0; j < 8; ++j) {
                if (bits[i + j] != 0) {
                    eightZeros = false;
                    break;
                }
            }
        }

        if (eightZeros) {
            // B8ZS substitution: 000VB0VB.
            // V has the same polarity as the reference pulse.
            // B has the opposite polarity.
            const Level V = lastPulse;
            const Level B = -lastPulse;

            output.insert(
                output.end(),
                {0.0, 0.0, 0.0, V, B, 0.0, B, V}
            );

            // The last pulse in the substitution is V.
            lastPulse = V;
            i += 8;
            continue;
        }

        if (bits[i] == 0) {
            output.push_back(0.0);
        } else {
            lastPulse = -lastPulse;
            output.push_back(lastPulse);
        }

        ++i;
    }

    return output;
}

std::string B8ZS::name() const {
    return "B8ZS";
}

} // namespace sigcodec
