#include "sigcodec/linecode/Manchester.hpp"

namespace sigcodec {

Signal Manchester::encode(const BitStream& bits) const
{
    Signal signal;

    double time = 0.0;

    for (Bit bit : bits) {
        Level firstLevel;
        Level secondLevel;

        if (bit == 1) {
            firstLevel = levels::Negative;
            secondLevel = levels::Positive;
        } else {
            firstLevel = levels::Positive;
            secondLevel = levels::Negative;
        }

        // First half of the bit.
        signal.push_back({time, firstLevel});
        signal.push_back({time + 0.5, firstLevel});

        // Mid-bit transition.
        signal.push_back({time + 0.5, secondLevel});
        signal.push_back({time + 1.0, secondLevel});

        time += 1.0;
    }

    return signal;
}

std::string Manchester::name() const
{
    return "Manchester";
}

} // namespace sigcodec