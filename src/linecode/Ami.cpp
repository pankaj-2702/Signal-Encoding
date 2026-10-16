#include "sigcodec/linecode/Ami.hpp"

namespace sigcodec {

Signal Ami::encode(const BitStream& bits) const
{
    Signal signal;

    double time = 0.0;
    Level nextPulse = levels::Positive;

    for (Bit bit : bits) {
        Level level = levels::Zero;

        if (bit == 1) {
            level = nextPulse;
            nextPulse = -nextPulse;
        }

        // Start and end of this bit period.
        signal.push_back({time, level});
        signal.push_back({time + 1.0, level});

        time += 1.0;
    }

    return signal;
}

std::string Ami::name() const
{
    return "AMI";
}

} // namespace sigcodec