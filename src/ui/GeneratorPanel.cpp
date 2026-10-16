
#include "sigcodec/ui/GeneratorPanel.hpp"

#include <algorithm>
#include <stdexcept>

namespace sigcodec {

GeneratorPanel::GeneratorPanel(const sf::Font& font)
    : font_(font),
      lengthText_("20"),
      lengthFocused_(false),
      useFixedPattern_(false),
      zeroCount_(8),
      status_("Ready"),
      lengthBounds_(320.f, 80.f, 100.f, 38.f),
      modeBounds_(440.f, 80.f, 230.f, 38.f),
      patternBounds_(690.f, 80.f, 150.f, 38.f),
      generateBounds_(855.f, 80.f, 120.f, 38.f) {
    generate();
}

void GeneratorPanel::drawText(
    sf::RenderTarget& target,
    const std::string& value,
    unsigned int size,
    float x,
    float y,
    const sf::Color& color
) const {
    sf::Text text(value, font_, size);
    text.setFillColor(color);
    text.setPosition(x, y);
    target.draw(text);
}

void GeneratorPanel::draw(sf::RenderTarget& target) const {
    const sf::Color dark(35, 45, 60);
    const sf::Color muted(100, 110, 125);
    const sf::Color blue(45, 105, 190);
    const sf::Color border(180, 190, 205);
    const sf::Color pale(242, 245, 249);

    drawText(target, "Digital Data Generator", 20, 320, 35, dark);

    drawText(target, "Length", 14, 320, 60, muted);

    sf::RectangleShape lengthBox(
        {lengthBounds_.width, lengthBounds_.height}
    );
    lengthBox.setPosition(lengthBounds_.left, lengthBounds_.top);
    lengthBox.setFillColor(sf::Color::White);
    lengthBox.setOutlineThickness(lengthFocused_ ? 2.f : 1.f);
    lengthBox.setOutlineColor(lengthFocused_ ? blue : border);
    target.draw(lengthBox);

    drawText(
        target,
        lengthText_,
        16,
        lengthBounds_.left + 10,
        lengthBounds_.top + 8,
        dark
    );

    drawText(target, "Generation mode", 14, 440, 60, muted);

    sf::RectangleShape modeBox({modeBounds_.width, modeBounds_.height});
    modeBox.setPosition(modeBounds_.left, modeBounds_.top);
    modeBox.setFillColor(pale);
    modeBox.setOutlineThickness(1.f);
    modeBox.setOutlineColor(border);
    target.draw(modeBox);

    drawText(
        target,
        useFixedPattern_ ? "Random + Pattern" : "Completely Random",
        14,
        modeBounds_.left + 8,
        modeBounds_.top + 9,
        dark
    );

    drawText(target, "Pattern", 14, 690, 60, muted);

    sf::RectangleShape patternBox(
        {patternBounds_.width, patternBounds_.height}
    );
    patternBox.setPosition(patternBounds_.left, patternBounds_.top);
    patternBox.setFillColor(useFixedPattern_ ? sf::Color(225, 237, 252) : pale);
    patternBox.setOutlineThickness(1.f);
    patternBox.setOutlineColor(border);
    target.draw(patternBox);

    drawText(
        target,
        std::to_string(zeroCount_) + " zeros",
        14,
        patternBounds_.left + 12,
        patternBounds_.top + 9,
        dark
    );

    sf::RectangleShape generateBox(
        {generateBounds_.width, generateBounds_.height}
    );
    generateBox.setPosition(generateBounds_.left, generateBounds_.top);
    generateBox.setFillColor(blue);
    target.draw(generateBox);

    drawText(
        target,
        "Generate",
        14,
        generateBounds_.left + 15,
        generateBounds_.top + 9,
        sf::Color::White
    );

    drawText(target, "Generated bits:", 15, 320, 135, muted);

    std::string bitString;
    bitString.reserve(result_.bits.size());

    for (Bit bit : result_.bits) {
        bitString += (bit == 0 ? '0' : '1');
    }

    drawText(
        target,
        bitString,
        16,
        320,
        158,
        dark
    );

    std::string palindrome;
    for (Bit bit : result_.longestPalindrome) {
        palindrome += (bit == 0 ? '0' : '1');
    }

    drawText(
        target,
        "Longest palindrome (" +
            std::to_string(result_.longestPalindrome.size()) + " bits):",
        15,
        320,
        188,
        muted
    );

    drawText(target, palindrome, 16, 320, 211, blue);
    drawText(target, status_, 13, 320, 238, muted);
}

void GeneratorPanel::handleEvent(const sf::Event& event) {
    if (event.type == sf::Event::MouseButtonPressed &&
        event.mouseButton.button == sf::Mouse::Left) {

        const float x = static_cast<float>(event.mouseButton.x);
        const float y = static_cast<float>(event.mouseButton.y);

        lengthFocused_ = lengthBounds_.contains(x, y);

        if (modeBounds_.contains(x, y)) {
            useFixedPattern_ = !useFixedPattern_;
        } else if (patternBounds_.contains(x, y)) {
            zeroCount_ = (zeroCount_ == 8) ? 4 : 8;
        } else if (generateBounds_.contains(x, y)) {
            generate();
        }
    }

    if (event.type == sf::Event::TextEntered && lengthFocused_) {
        if (event.text.unicode == 8) {
            if (!lengthText_.empty()) {
                lengthText_.pop_back();
            }
        } else if (event.text.unicode >= '0' &&
                   event.text.unicode <= '9') {
            if (lengthText_.size() < 6) {
                lengthText_ += static_cast<char>(event.text.unicode);
            }
        }
    }
}

void GeneratorPanel::generate() {
    try {
        if (lengthText_.empty()) {
            throw std::invalid_argument("Enter a sequence length.");
        }

        const unsigned long parsed = std::stoul(lengthText_);

        if (parsed == 0 || parsed > 100000) {
            throw std::invalid_argument(
                "Length must be between 1 and 100000."
            );
        }

        const std::size_t length = static_cast<std::size_t>(parsed);

        if (useFixedPattern_) {
            if (zeroCount_ > length) {
                throw std::invalid_argument(
                    "Length must be at least " +
                    std::to_string(zeroCount_) + "."
                );
            }

            result_ = generator_.generateWithPattern(
                length,
                BitStream(zeroCount_, 0)
            );
        } else {
            result_ = generator_.generateRandom(length);
        }

        status_ = useFixedPattern_
            ? "Inserted " + std::to_string(zeroCount_) +
                  " zeros at index " +
                  std::to_string(result_.patternStart) + "."
            : "Generated a completely random sequence.";
    } catch (const std::exception& error) {
        status_ = error.what();
    }
}

const BitStream& GeneratorPanel::bits() const {
    return result_.bits;
}

const BitStream& GeneratorPanel::longestPalindrome() const {
    return result_.longestPalindrome;
}

} // namespace sigcodec
