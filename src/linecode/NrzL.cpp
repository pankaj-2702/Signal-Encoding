#include "sigcodec/linecode/NrzL.hpp"
#include "sigcodec/core/Conventions.hpp"


namespace sigcodec {

Signal NrzL::encode(const BitStream& bits) const
{
    Signal signal;

    double time = 0.0;

    for (Bit bit : bits) {

        Level level;

        if (bit == 1) {
            level = levels::Positive;
        } else {
            level = levels::Negative;
        }

        // Start of the bit period
        signal.push_back({time, level});

        // End of the bit period
        signal.push_back({time + 1.0, level});

        time += 1.0;
    }

    return signal;
}


std::string NrzL::name() const
{
    return "NRZ-L";
}

} // namespace sigcodec