#pragma once

#include "sigcodec/linecode/ILineCoder.hpp"
#include "sigcodec/core/Conventions.hpp"

namespace sigcodec {

class Ami : public ILineCoder {
public:
    Signal encode(const BitStream& bits) const override;

    std::string name() const override;
};

} // namespace sigcodec