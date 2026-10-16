#include "sigcodec/ui/Menu.hpp"
#include <algorithm>
namespace sigcodec {

Menu::Menu(const sf::Font& font)
    : font_(font),
      encoders_{
          "NRZ-L",
          "NRZ-I",
          "Manchester",
          "Differential Manchester",
          "AMI"
      },
      selectedEncoder_("NRZ-L"),
      menuBounds_(20.0f, 80.0f, 250.0f, 45.0f)
{
}

void Menu::draw(sf::RenderTarget& target) const
{
    sf::Text heading("Encoding Technique", font_, 20);
    heading.setFillColor(sf::Color(35, 35, 35));
    heading.setPosition(20.0f, 35.0f);
    target.draw(heading);

    sf::RectangleShape background({
        menuBounds_.width,
        menuBounds_.height
    });

    background.setPosition(menuBounds_.left, menuBounds_.top);
    background.setFillColor(sf::Color(242, 245, 249));
    background.setOutlineThickness(1.0f);
    background.setOutlineColor(sf::Color(170, 180, 195));

    target.draw(background);

    sf::Text selected(selectedEncoder_, font_, 17);
    selected.setFillColor(sf::Color(30, 40, 55));
    selected.setPosition(
        menuBounds_.left + 12.0f,
        menuBounds_.top + 9.0f
    );

    target.draw(selected);

    sf::Text arrow("v", font_, 16);
    arrow.setFillColor(sf::Color(30, 40, 55));
    arrow.setPosition(
        menuBounds_.left + menuBounds_.width - 25.0f,
        menuBounds_.top + 10.0f
    );

    target.draw(arrow);
}

void Menu::handleClick(float x, float y)
{
    if (!menuBounds_.contains(x, y)) {
        return;
    }

    auto it = std::find(
        encoders_.begin(),
        encoders_.end(),
        selectedEncoder_
    );

    if (it == encoders_.end()) {
        selectedEncoder_ = encoders_.front();
        return;
    }

    ++it;

    if (it == encoders_.end()) {
        it = encoders_.begin();
    }

    selectedEncoder_ = *it;
}

const std::string& Menu::selectedEncoder() const
{
    return selectedEncoder_;
}

} // namespace sigcodec