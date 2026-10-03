#pragma once

#include "sigcodec/linecode/ILineCoder.hpp"

namespace sigcodec {

class NrzL : public ILineCoder {
public:

    Signal encode(const BitStream& bits) const override;

    std::string name() const override;
};

} // namespace sigcodec