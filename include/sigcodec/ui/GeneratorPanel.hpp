
#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <cstddef>

#include "sigcodec/generator/DataGenerator.hpp"

namespace sigcodec {

enum class GeneratorMode {
    Random = 0,
    RandomWithPattern,
    Custom
};

class GeneratorPanel {
public:
    explicit GeneratorPanel(const sf::Font& font, const sf::Font* boldFont = nullptr);

    void draw(sf::RenderTarget& target) const;
    void handleEvent(const sf::Event& event, float windowWidth);

    const BitStream& bits() const;
    const BitStream& longestPalindrome() const;

    bool isInputFocused() const {
        return lengthFocused_ || customFocused_;
    }

private:
    const sf::Font& font_;
    const sf::Font* boldFont_;
    DataGenerator generator_;
    GenerationResult result_;

    GeneratorMode mode_;
    std::string lengthText_;
    bool lengthFocused_;
    std::string customBitsText_;
    bool customFocused_;
    std::size_t zeroCount_;
    std::string status_;

    mutable sf::FloatRect lengthBounds_;
    mutable sf::FloatRect customInputBounds_;
    mutable sf::FloatRect modeBounds_;
    mutable sf::FloatRect patternBounds_;
    mutable sf::FloatRect generateBounds_;

    void generate();

    const sf::Font& labelFont() const {
        return boldFont_ ? *boldFont_ : font_;
    }

    void drawText(
        sf::RenderTarget& target,
        const std::string& value,
        unsigned int size,
        float x,
        float y,
        const sf::Color& color,
        bool bold = false
    ) const;
};

} // namespace sigcodec
