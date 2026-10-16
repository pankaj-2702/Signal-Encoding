
#pragma once

#include "sigcodec/linecode/IZeroScrambler.hpp"

namespace sigcodec {

class HDB3 : public IZeroScrambler {
public:
    BipolarSignal encode(const BitStream& bits) const override;
    std::string name() const override;
};

} // namespace sigcodec
