#include "sigcodec/linecode/DiffManchester.hpp"

namespace sigcodec {

Signal DiffManchester::encode(const BitStream& bits) const
{
    Signal signal;

    double time = 0.0;
    Level currentLevel = levels::Positive;

    for (Bit bit : bits) {

        // A zero causes a transition at the beginning.
        if (bit == 0) {
            currentLevel = -currentLevel;
        }

        // First half of the bit.
        signal.push_back({time, currentLevel});
        signal.push_back({time + 0.5, currentLevel});

        // Mandatory mid-bit transition.
        currentLevel = -currentLevel;

        // Second half of the bit.
        signal.push_back({time + 0.5, currentLevel});
        signal.push_back({time + 1.0, currentLevel});

        time += 1.0;
    }

    return signal;
}

std::string DiffManchester::name() const
{
    return "Differential Manchester";
}

} // namespace sigcodec