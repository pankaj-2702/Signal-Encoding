
#pragma once

#include <SFML/Graphics.hpp>
#include <string>

#include "sigcodec/generator/DataGenerator.hpp"

namespace sigcodec {

class GeneratorPanel {
public:
    explicit GeneratorPanel(const sf::Font& font);

    void draw(sf::RenderTarget& target) const;
    void handleEvent(const sf::Event& event);

    const BitStream& bits() const;
    const BitStream& longestPalindrome() const;

private:
    const sf::Font& font_;
    DataGenerator generator_;
    GenerationResult result_;

    std::string lengthText_;
    bool lengthFocused_;
    bool useFixedPattern_;
    std::size_t zeroCount_;
    std::string status_;

    sf::FloatRect lengthBounds_;
    sf::FloatRect modeBounds_;
    sf::FloatRect patternBounds_;
    sf::FloatRect generateBounds_;

    void generate();
    void drawText(
        sf::RenderTarget& target,
        const std::string& value,
        unsigned int size,
        float x,
        float y,
        const sf::Color& color
    ) const;
};

} // namespace sigcodec
