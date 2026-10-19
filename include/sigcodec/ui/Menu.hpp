
#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

namespace sigcodec {

class Menu {
public:
    explicit Menu(const sf::Font& font, const sf::Font* boldFont = nullptr);

    void draw(sf::RenderTarget& target) const;

    // x and y must be window coordinates.
    void handleClick(float x, float y);

    const std::string& selectedEncoder() const;
    void setSelectedEncoder(const std::string& encoder);

    char scramblingChoice() const;
    void setScramblingChoice(char choice);

private:
    const sf::Font& font_;
    const sf::Font* boldFont_;

    std::vector<std::string> encoders_;
    std::string selectedEncoder_;
    char scramblingChoice_;

    // Cached button bounds for click detection
    mutable std::vector<std::pair<std::string, sf::FloatRect>> encoderButtons_;
    mutable std::vector<std::pair<char, sf::FloatRect>> scramblingButtons_;

    const sf::Font& labelFont() const {
        return boldFont_ ? *boldFont_ : font_;
    }
};

} // namespace sigcodec

