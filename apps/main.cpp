#include <iostream>

#include "sigcodec/core/Types.hpp"
#include "sigcodec/linecode/NrzL.hpp"
#include "sigcodec/linecode/NrzI.hpp"

int main()
{
    using namespace sigcodec;

    BitStream bits = {
        1, 0, 1, 1, 0
    };

    NrzI encoder;

    Signal signal = encoder.encode(bits);

    std::cout << "Encoder: "
              << encoder.name()
              << "\n\n";

    std::cout << "Input bits: ";

    for (Bit bit : bits) {
        std::cout << static_cast<int>(bit);
    }

    std::cout << "\n\n";

    std::cout << "Generated signal:\n";

    for (const auto& point : signal) {
        std::cout
            << "t = " << point.time
            << ", level = " << point.level
            << '\n';
    }

    return 0;
}