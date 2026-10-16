#pragma once

#include <SFML/Graphics.hpp>
#include <string>

#include "sigcodec/core/Types.hpp"

namespace sigcodec {

class SignalPlotter {
public:
    void draw(
        sf::RenderTarget& target,
        const Signal& signal,
        const BitStream& bits,
        const std::string& title,
        float x,
        float y,
        float width,
        float height,
        const sf::Color& color,
        const sf::Font& font
    ) const;
};

} // namespace sigcodec