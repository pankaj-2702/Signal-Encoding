#include "sigcodec/linecode/NrzI.hpp"

namespace sigcodec {

Signal NrzI::encode(const BitStream& bits) const
{
    Signal signal;

    double time = 0.0;

    // Initial signal level.
    Level currentLevel = levels::Negative;

    for (Bit bit : bits) {

        // In NRZ-I, a 1 causes a transition.
        if (bit == 1) {
            currentLevel = -currentLevel;
        }

        // Start of the bit period.
        signal.push_back({time, currentLevel});

        // End of the bit period.
        signal.push_back({time + 1.0, currentLevel});

        time += 1.0;
    }

    return signal;
}


std::string NrzI::name() const
{
    return "NRZ-I";
}

} // namespace sigcodec