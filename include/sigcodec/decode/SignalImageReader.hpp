
#pragma once

#include <string>

#include "sigcodec/core/Types.hpp"

namespace sigcodec {

class IZeroScrambler {
public:
    virtual ~IZeroScrambler() = default;

    virtual BitStream encode(const BitStream& bits) const = 0;
    virtual std::string name() const = 0;
};

} // namespace sigcodec
