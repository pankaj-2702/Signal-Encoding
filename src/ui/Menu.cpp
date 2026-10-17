#include "sigcodec/ui/Menu.hpp"

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
      menuBounds_(20.f, 80.f, 250.f, 45.f),
      isOpen_(false),
      optionHeight_(30.f) {
}

void Menu::draw(sf::RenderTarget& target) const {
    const sf::Color background(242, 245, 249);
    const sf::Color border(170, 180, 195);
    const sf::Color textColor(35, 45, 60);
    const sf::Color selectedColor(220, 235, 255);

    sf::Text heading("Encoding Technique", font_, 20);
    heading.setFillColor(textColor);
    heading.setPosition(20.f, 35.f);
    target.draw(heading);

    sf::RectangleShape box(
        sf::Vector2f(menuBounds_.width, menuBounds_.height)
    );
    box.setPosition(menuBounds_.left, menuBounds_.top);
    box.setFillColor(background);
    box.setOutlineThickness(1.f);
    box.setOutlineColor(border);
    target.draw(box);

    sf::Text selected(selectedEncoder_, font_, 17);
    selected.setFillColor(textColor);
    selected.setPosition(menuBounds_.left + 12.f,
                         menuBounds_.top + 9.f);
    target.draw(selected);

    sf::Text arrow(isOpen_ ? "^" : "v", font_, 16);
    arrow.setFillColor(textColor);
    arrow.setPosition(menuBounds_.left + menuBounds_.width - 25.f,
                      menuBounds_.top + 10.f);
    target.draw(arrow);

    if (isOpen_) {
        for (std::size_t i = 0; i < encoders_.size(); ++i) {
            float optionY = menuBounds_.top + menuBounds_.height
                          + static_cast<float>(i) * optionHeight_;

            sf::RectangleShape option(
                sf::Vector2f(menuBounds_.width, optionHeight_)
            );
            option.setPosition(menuBounds_.left, optionY);
            option.setFillColor(
                encoders_[i] == selectedEncoder_
                    ? selectedColor : background
            );
            option.setOutlineThickness(1.f);
            option.setOutlineColor(border);
            target.draw(option);

            sf::Text optionText(encoders_[i], font_, 15);
            optionText.setFillColor(textColor);
            optionText.setPosition(menuBounds_.left + 12.f,
                                   optionY + 6.f);
            target.draw(optionText);
        }
    }
}

void Menu::handleClick(float x, float y) {
    if (menuBounds_.contains(x, y)) {
        isOpen_ = !isOpen_;
        return;
    }

    if (!isOpen_) {
        return;
    }

    float optionsStartY = menuBounds_.top + menuBounds_.height;

    if (x >= menuBounds_.left &&
        x <= menuBounds_.left + menuBounds_.width &&
        y >= optionsStartY) {

        int index = static_cast<int>(
            (y - optionsStartY) / optionHeight_
        );

        if (index >= 0 &&
            index < static_cast<int>(encoders_.size())) {
            selectedEncoder_ = encoders_[index];
            isOpen_ = false;
            return;
        }
    }

    isOpen_ = false;
}

const std::string& Menu::selectedEncoder() const {
    return selectedEncoder_;
}

}
