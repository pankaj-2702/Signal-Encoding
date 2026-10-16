
#pragma once

#include "sigcodec/linecode/IZeroScrambler.hpp"

namespace sigcodec {

class B8ZS : public IZeroScrambler {
public:
    BipolarSignal encode(const BitStream& bits) const override;
    std::string name() const override;
};

} // namespace sigcodec
