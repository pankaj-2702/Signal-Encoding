#pragma once

#include <string>

#include "sigcodec/core/Types.hpp"

namespace sigcodec {

class ILineCoder {
public:

    virtual ~ILineCoder() = default;

    // Encode a binary data stream into a digital signal.
    virtual Signal encode(const BitStream& bits) const = 0;

    // Return the name of the line-coding scheme.
    virtual std::string name() const = 0;
};

} // namespace sigcodec