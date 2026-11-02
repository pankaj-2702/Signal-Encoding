#include "sigcodec/ui/GeneratorPanel.hpp"

#include <algorithm>
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace sigcodec {

GeneratorPanel::GeneratorPanel(const sf::Font& font, const sf::Font* boldFont)
    : font_(font),
      boldFont_(boldFont),
      mode_(GeneratorMode::Random),
      lengthText_("20"),
      lengthFocused_(false),
      customBitsText_("10110000000010"),
      customFocused_(false),
      zeroCount_(8),
      status_("Ready"),
      lengthBounds_(0.f, 0.f, 0.f, 0.f),
      customInputBounds_(0.f, 0.f, 0.f, 0.f),
      modeBounds_(0.f, 0.f, 0.f, 0.f),
      patternBounds_(0.f, 0.f, 0.f, 0.f),
      generateBounds_(0.f, 0.f, 0.f, 0.f)
{
    generate();
}

void GeneratorPanel::drawText(
    sf::RenderTarget& target,
    const std::string& value,
    unsigned int size,
    float x,
    float y,
    const sf::Color& color,
    bool bold
) const
{
    sf::Text text(value, bold ? labelFont() : font_, size);
    if (bold) {
        text.setStyle(sf::Text::Bold);
    }
    text.setFillColor(color);
    text.setPosition(x, y);
    target.draw(text);
}

void GeneratorPanel::draw(sf::RenderTarget& target) const
{
    const float windowWidth = static_cast<float>(target.getSize().x);
    const float margin = 36.f;

    const float availW = windowWidth - 2.f * margin;
    const float card1Width = std::max(520.f, std::min(700.f, availW * 0.38f));
    const float cardX = margin + card1Width + 24.f;
    const float cardY = 125.f;
    const float cardWidth = windowWidth - cardX - margin;
    const float cardHeight = 390.f;

    // Outer card container
    sf::RectangleShape card({cardWidth, cardHeight});
    card.setPosition(cardX, cardY);
    card.setFillColor(sf::Color(255, 255, 255));
    card.setOutlineThickness(2.0f);
    card.setOutlineColor(sf::Color(226, 232, 240));
    target.draw(card);

    // Top accent line
    sf::RectangleShape topAccent({cardWidth, 5.f});
    topAccent.setPosition(cardX, cardY);
    topAccent.setFillColor(sf::Color(99, 102, 241)); // Indigo accent
    target.draw(topAccent);

    // Card Header (3x larger bold font)
    drawText(target, "DIGITAL DATA GENERATOR", 36, cardX + 24.f, cardY + 16.f, sf::Color(15, 23, 42), true);
    drawText(target, "Configure bitstream generation or type custom binary input", 22, cardX + 24.f, cardY + 60.f, sf::Color(100, 116, 139), false);

    // Control bounds calculation
    const float ctrlY = cardY + 128.f;
    const float ctrlH = 50.f;
    const float gap = 14.f;

    const float modeW = 240.f;
    const float inputW = 350.f;
    const float patW = 220.f;
    const float genW = 160.f;

    const float modeX = cardX + 24.f;
    const float inputX = modeX + modeW + gap;
    const float patX = inputX + inputW + gap;
    const float genX = patX + patW + gap;

    modeBounds_ = {modeX, ctrlY, modeW, ctrlH};
    lengthBounds_ = {inputX, ctrlY, inputW, ctrlH};
    customInputBounds_ = {inputX, ctrlY, inputW, ctrlH};
    patternBounds_ = {patX, ctrlY, patW, ctrlH};
    generateBounds_ = {genX, ctrlY, genW, ctrlH};

    // Control Labels
    drawText(target, "Generation Mode:", 22, modeX, cardY + 98.f, sf::Color(71, 85, 105), true);
    const std::string inputLabel = (mode_ == GeneratorMode::Custom) ? "Custom Bits (0/1):" : "Length (bits):";
    drawText(target, inputLabel, 22, inputX, cardY + 98.f, sf::Color(71, 85, 105), true);
    const std::string patternLabel = (mode_ == GeneratorMode::Custom) ? "Quick Template:" : "Zero Pattern:";
    drawText(target, patternLabel, 22, patX, cardY + 98.f, sf::Color(71, 85, 105), true);
    drawText(target, "Action:", 22, genX, cardY + 98.f, sf::Color(71, 85, 105), true);

    auto drawButton = [&](const sf::FloatRect& bounds,
                          const std::string& text,
                          const sf::Color& fill,
                          const sf::Color& outline,
                          const sf::Color& textColor,
                          unsigned int fontSize,
                          bool boldText) {
        sf::RectangleShape box({bounds.width, bounds.height});
        box.setPosition(bounds.left, bounds.top);
        box.setFillColor(fill);
        box.setOutlineThickness(1.5f);
        box.setOutlineColor(outline);
        target.draw(box);

        sf::Text t(text, boldText ? labelFont() : font_, fontSize);
        if (boldText) {
            t.setStyle(sf::Text::Bold);
        }
        t.setFillColor(textColor);
        const sf::FloatRect tb = t.getLocalBounds();
        t.setPosition(
            bounds.left + (bounds.width - tb.width) / 2.f - tb.left,
            bounds.top + (bounds.height - tb.height) / 2.f - tb.top - 2.f
        );
        target.draw(t);
    };

    // 1. Mode toggle button
    std::string modeText = "Completely Random";
    if (mode_ == GeneratorMode::RandomWithPattern) {
        modeText = "Random + Pattern";
    } else if (mode_ == GeneratorMode::Custom) {
        modeText = "Custom Input";
    }

    drawButton(
        modeBounds_,
        modeText,
        (mode_ != GeneratorMode::Random) ? sf::Color(238, 242, 255) : sf::Color(241, 245, 249),
        (mode_ != GeneratorMode::Random) ? sf::Color(99, 102, 241) : sf::Color(203, 213, 225),
        (mode_ != GeneratorMode::Random) ? sf::Color(67, 56, 202) : sf::Color(51, 65, 85),
        22,
        true
    );

    // 2. Input box (Length or Custom Bits)
    {
        const bool isFocused = (mode_ == GeneratorMode::Custom) ? customFocused_ : lengthFocused_;
        sf::RectangleShape box({inputW, ctrlH});
        box.setPosition(inputX, ctrlY);
        box.setFillColor(sf::Color(255, 255, 255));
        box.setOutlineThickness(isFocused ? 2.5f : 1.5f);
        box.setOutlineColor(isFocused ? sf::Color(37, 99, 235) : sf::Color(203, 213, 225));
        target.draw(box);

        if (mode_ == GeneratorMode::Custom) {
            std::string display;
            sf::Color textColor = sf::Color(15, 23, 42);

            if (customBitsText_.empty() && !customFocused_) {
                display = "Click here to type 0/1...";
                textColor = sf::Color(148, 163, 184);
            } else {
                if (customBitsText_.size() <= 20) {
                    display = customBitsText_ + (customFocused_ ? "|" : "");
                } else {
                    display = "..." + customBitsText_.substr(customBitsText_.size() - 17) + (customFocused_ ? "|" : "");
                }
            }

            sf::Text inText(display, labelFont(), 24);
            inText.setStyle(sf::Text::Bold);
            inText.setFillColor(textColor);
            const sf::FloatRect tb = inText.getLocalBounds();
            inText.setPosition(
                inputX + 16.f - tb.left,
                ctrlY + (ctrlH - tb.height) / 2.f - tb.top - 2.f
            );
            target.draw(inText);
        } else {
            std::string display = lengthText_ + (lengthFocused_ ? "|" : "");
            sf::Text lenT(display, labelFont(), 26);
            lenT.setStyle(sf::Text::Bold);
            lenT.setFillColor(sf::Color(15, 23, 42));
            const sf::FloatRect tb = lenT.getLocalBounds();
            lenT.setPosition(
                inputX + (inputW - tb.width) / 2.f - tb.left,
                ctrlY + (ctrlH - tb.height) / 2.f - tb.top - 2.f
            );
            target.draw(lenT);
        }
    }

    // 3. Pattern / Template button
    if (mode_ == GeneratorMode::Custom) {
        drawButton(
            patternBounds_,
            "+8 Zeros (B8ZS)",
            sf::Color(241, 245, 249),
            sf::Color(148, 163, 184),
            sf::Color(30, 41, 59),
            22,
            true
        );
    } else {
        std::string patText = std::to_string(zeroCount_) + " Zeros " + (zeroCount_ == 8 ? "(B8ZS)" : "(HDB3)");
        drawButton(
            patternBounds_,
            patText,
            (mode_ == GeneratorMode::RandomWithPattern) ? sf::Color(241, 245, 249) : sf::Color(248, 250, 252),
            (mode_ == GeneratorMode::RandomWithPattern) ? sf::Color(148, 163, 184) : sf::Color(226, 232, 240),
            (mode_ == GeneratorMode::RandomWithPattern) ? sf::Color(30, 41, 59) : sf::Color(148, 163, 184),
            22,
            true
        );
    }

    // 4. Action button (GENERATE / APPLY)
    const std::string actionText = (mode_ == GeneratorMode::Custom) ? "> APPLY" : "> GENERATE";
    drawButton(
        generateBounds_,
        actionText,
        sf::Color(37, 99, 235),
        sf::Color(29, 78, 216),
        sf::Color(255, 255, 255),
        24,
        true
    );

    // Horizontal Divider
    sf::RectangleShape divider({cardWidth - 48.f, 1.5f});
    divider.setPosition(cardX + 24.f, cardY + 195.f);
    divider.setFillColor(sf::Color(226, 232, 240));
    target.draw(divider);

    // Lower Section: Generated Bits (Col 1) and Palindrome (Col 2)
    const float innerW = cardWidth - 48.f;
    const float col1W = std::min(innerW * 0.58f, innerW - 320.f);
    const float col2W = innerW - col1W - 24.f;
    const float col2X = cardX + 24.f + col1W + 24.f;

    // Col 1: Generated bits
    std::string bitsHeader = "CURRENT BITSTREAM (N = " + std::to_string(result_.bits.size()) + "):";
    drawText(target, bitsHeader, 22, cardX + 24.f, cardY + 208.f, sf::Color(71, 85, 105), true);

    sf::RectangleShape bitsBox({col1W, 85.f});
    bitsBox.setPosition(cardX + 24.f, cardY + 240.f);
    bitsBox.setFillColor(sf::Color(248, 250, 252));
    bitsBox.setOutlineThickness(1.5f);
    bitsBox.setOutlineColor(sf::Color(226, 232, 240));
    target.draw(bitsBox);

    std::string bitString;
    bitString.reserve(result_.bits.size());
    for (Bit b : result_.bits) {
        bitString += (b == 0 ? '0' : '1');
    }

    const std::size_t charsPerLine = std::max<std::size_t>(10, static_cast<std::size_t>(col1W / 17.f));
    if (bitString.size() <= charsPerLine) {
        drawText(target, bitString, 24, cardX + 36.f, cardY + 266.f, sf::Color(15, 23, 42), true);
    } else {
        std::string line1 = bitString.substr(0, charsPerLine);
        drawText(target, line1, 24, cardX + 36.f, cardY + 250.f, sf::Color(15, 23, 42), true);

        if (bitString.size() <= charsPerLine * 2) {
            std::string line2 = bitString.substr(charsPerLine);
            drawText(target, line2, 24, cardX + 36.f, cardY + 284.f, sf::Color(15, 23, 42), true);
        } else {
            std::string line2 = bitString.substr(charsPerLine, charsPerLine - 4) + "...";
            drawText(target, line2, 24, cardX + 36.f, cardY + 284.f, sf::Color(15, 23, 42), true);
            drawText(target, "(+" + std::to_string(bitString.size() - 2 * charsPerLine + 4) + " more bits)", 18, cardX + 36.f, cardY + 314.f, sf::Color(148, 163, 184), false);
        }
    }

    // Status line
    drawText(target, "Status: " + status_, 20, cardX + 24.f, cardY + 344.f, sf::Color(100, 116, 139), false);

    // Col 2: Longest Palindrome
    drawText(target, "LONGEST PALINDROME:", 22, col2X, cardY + 208.f, sf::Color(71, 85, 105), true);

    sf::RectangleShape palBox({col2W, 85.f});
    palBox.setPosition(col2X, cardY + 240.f);
    palBox.setFillColor(sf::Color(238, 242, 255));
    palBox.setOutlineThickness(1.5f);
    palBox.setOutlineColor(sf::Color(199, 210, 254));
    target.draw(palBox);

    std::string palLength = "Length: " + std::to_string(result_.longestPalindrome.size()) + " bits";
    drawText(target, palLength, 20, col2X + 18.f, cardY + 250.f, sf::Color(67, 56, 202), true);

    std::string palString;
    for (Bit b : result_.longestPalindrome) {
        palString += (b == 0 ? '0' : '1');
    }

    if (palString.empty()) {
        palString = "None";
    } else if (palString.size() > 22) {
        palString = palString.substr(0, 20) + "...";
    }

    drawText(target, palString, 28, col2X + 18.f, cardY + 280.f, sf::Color(79, 70, 229), true);
}

void GeneratorPanel::handleEvent(
    const sf::Event& event,
    float windowWidth
)
{
    (void)windowWidth;

    if (event.type == sf::Event::MouseButtonPressed &&
        event.mouseButton.button == sf::Mouse::Left) {

        const float x = static_cast<float>(event.mouseButton.x);
        const float y = static_cast<float>(event.mouseButton.y);

        if (mode_ == GeneratorMode::Custom) {
            customFocused_ = customInputBounds_.contains(x, y);
            lengthFocused_ = false;
        } else {
            lengthFocused_ = lengthBounds_.contains(x, y);
            customFocused_ = false;
        }

        if (modeBounds_.contains(x, y)) {
            if (mode_ == GeneratorMode::Random) {
                mode_ = GeneratorMode::RandomWithPattern;
                customFocused_ = false;
                lengthFocused_ = false;
            } else if (mode_ == GeneratorMode::RandomWithPattern) {
                mode_ = GeneratorMode::Custom;
                customFocused_ = true;
                lengthFocused_ = false;
            } else {
                mode_ = GeneratorMode::Random;
                customFocused_ = false;
                lengthFocused_ = false;
            }
            generate();
        } else if (patternBounds_.contains(x, y)) {
            if (mode_ == GeneratorMode::Custom) {
                // Quick template: append 8 zeros for B8ZS scrambling testing
                customBitsText_ += "00000000";
                if (customBitsText_.size() > 64) {
                    customBitsText_ = customBitsText_.substr(0, 64);
                }
                generate();
            } else {
                zeroCount_ = (zeroCount_ == 8) ? 4 : 8;
                if (mode_ == GeneratorMode::RandomWithPattern) {
                    generate();
                }
            }
        } else if (generateBounds_.contains(x, y)) {
            generate();
        }
    }

    if (event.type == sf::Event::TextEntered) {
        if (mode_ == GeneratorMode::Custom && customFocused_) {
            if (event.text.unicode == 8) { // Backspace
                if (!customBitsText_.empty()) {
                    customBitsText_.pop_back();
                    if (!customBitsText_.empty()) {
                        generate();
                    } else {
                        status_ = "Enter binary bits (0 or 1).";
                    }
                }
            } else if (event.text.unicode == '0' || event.text.unicode == '1') {
                if (customBitsText_.size() < 64) {
                    customBitsText_ += static_cast<char>(event.text.unicode);
                    generate();
                }
            } else if (event.text.unicode == 13) { // Enter key
                customFocused_ = false;
                generate();
            }
        } else if (mode_ != GeneratorMode::Custom && lengthFocused_) {
            if (event.text.unicode == 8) { // Backspace
                if (!lengthText_.empty()) {
                    lengthText_.pop_back();
                }
            } else if (event.text.unicode >= '0' &&
                       event.text.unicode <= '9') {
                if (lengthText_.size() < 4) {
                    lengthText_ += static_cast<char>(event.text.unicode);
                }
            } else if (event.text.unicode == 13) { // Enter key
                lengthFocused_ = false;
                generate();
            }

            status_ = "Press Generate to apply the new length.";
        }
    }

    if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::Escape) {
            customFocused_ = false;
            lengthFocused_ = false;
        }
    }
}

void GeneratorPanel::generate()
{
    try {
        if (mode_ == GeneratorMode::Custom) {
            if (customBitsText_.empty()) {
                status_ = "Enter custom bits (0 or 1).";
                return;
            }
            result_ = generator_.createCustom(customBitsText_);
            status_ = "Custom bitstream applied (" + std::to_string(result_.bits.size()) + " bits).";
        } else {
            if (lengthText_.empty()) {
                throw std::invalid_argument("Enter a sequence length.");
            }

            const unsigned long parsed = std::stoul(lengthText_);

            if (parsed == 0 || parsed > 10000) {
                throw std::invalid_argument(
                    "Length must be between 1 and 10000."
                );
            }

            const std::size_t length = static_cast<std::size_t>(parsed);

            if (mode_ == GeneratorMode::RandomWithPattern) {
                if (zeroCount_ > length) {
                    throw std::invalid_argument(
                        "Length must be >= " + std::to_string(zeroCount_) + "."
                    );
                }

                result_ = generator_.generateWithPattern(
                    length,
                    BitStream(zeroCount_, 0)
                );
                status_ = "Inserted " + std::to_string(zeroCount_) +
                          " zeros at bit index " +
                          std::to_string(result_.patternStart) + ".";
            } else {
                result_ = generator_.generateRandom(length);
                status_ = "Generated a random sequence of " + std::to_string(length) + " bits.";
            }
        }
    } catch (const std::exception& error) {
        status_ = error.what();
    }
}

const BitStream& GeneratorPanel::bits() const
{
    return result_.bits;
}

const BitStream& GeneratorPanel::longestPalindrome() const
{
    return result_.longestPalindrome;
}

} // namespace sigcodec
