#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

namespace sigcodec {

class Menu {
public:
    explicit Menu(const sf::Font& font);

    void draw(sf::RenderTarget& target) const;

    void handleClick(float x, float y);

    const std::string& selectedEncoder() const;

private:
    const sf::Font& font_;

    std::vector<std::string> encoders_;

    std::string selectedEncoder_;

    sf::FloatRect menuBounds_;
};

} // namespace sigcodec