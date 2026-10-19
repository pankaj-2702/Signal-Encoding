#include "sigcodec/ui/Menu.hpp"

#include <algorithm>

namespace sigcodec {

Menu::Menu(const sf::Font& font, const sf::Font* boldFont)
    : font_(font),
      boldFont_(boldFont),
      encoders_{
          "NRZ-L",
          "NRZ-I",
          "Manchester",
          "Diff Manchester",
          "AMI"
      },
      selectedEncoder_("NRZ-L"),
      scramblingChoice_('N')
{
}

void Menu::draw(sf::RenderTarget& target) const
{
    const float windowWidth = static_cast<float>(target.getSize().x);
    const float margin = 36.f;

    const float availW = windowWidth - 2.f * margin;
    const float cardWidth = std::max(520.f, std::min(700.f, availW * 0.38f));
    const float cardHeight = 390.f;
    const float cardX = margin;
    const float cardY = 125.f;

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
    topAccent.setFillColor(sf::Color(37, 99, 235));
    target.draw(topAccent);

    // Card Header (3x larger bold font)
    sf::Text heading("ENCODING SCHEME", labelFont(), 36);
    heading.setStyle(sf::Text::Bold);
    heading.setFillColor(sf::Color(15, 23, 42));
    heading.setPosition(cardX + 24.f, cardY + 16.f);
    target.draw(heading);

    sf::Text subtitle("Select line coding standard", font_, 22);
    subtitle.setFillColor(sf::Color(100, 116, 139));
    subtitle.setPosition(cardX + 24.f, cardY + 60.f);
    target.draw(subtitle);

    encoderButtons_.clear();

    const float btnGap = 12.f;
    const float padSide = 24.f;
    const float btnW = (cardWidth - 2.f * padSide - btnGap) / 2.f;
    const float btnH = 50.f;
    const float startY = cardY + 98.f;

    struct BtnDef {
        std::string name;
        std::string label;
        float x;
        float y;
        float w;
        float h;
    };

    const std::vector<BtnDef> btnDefs = {
        {"NRZ-L", "NRZ-L", cardX + padSide, startY, btnW, btnH},
        {"NRZ-I", "NRZ-I", cardX + padSide + btnW + btnGap, startY, btnW, btnH},
        {"Manchester", "Manchester", cardX + padSide, startY + btnH + btnGap, btnW, btnH},
        {"Differential Manchester", "Diff Manchester", cardX + padSide + btnW + btnGap, startY + btnH + btnGap, btnW, btnH},
        {"AMI", "AMI (Bipolar Alternate)", cardX + padSide, startY + 2.f * (btnH + btnGap), cardWidth - 2.f * padSide, btnH}
    };

    for (const auto& def : btnDefs) {
        const bool isSelected = (selectedEncoder_ == def.name ||
                                (def.name == "Differential Manchester" && selectedEncoder_ == "Differential Manchester"));

        encoderButtons_.push_back({def.name, {def.x, def.y, def.w, def.h}});

        sf::RectangleShape btn({def.w, def.h});
        btn.setPosition(def.x, def.y);
        btn.setFillColor(isSelected ? sf::Color(37, 99, 235) : sf::Color(248, 250, 252));
        btn.setOutlineThickness(isSelected ? 2.5f : 1.5f);
        btn.setOutlineColor(isSelected ? sf::Color(29, 78, 216) : sf::Color(203, 213, 225));
        target.draw(btn);

        sf::Text label(def.label, labelFont(), 26);
        if (isSelected) {
            label.setStyle(sf::Text::Bold);
            label.setFillColor(sf::Color(255, 255, 255));
        } else {
            label.setFillColor(sf::Color(51, 65, 85));
        }

        const sf::FloatRect bounds = label.getLocalBounds();
        label.setPosition(
            def.x + (def.w - bounds.width) / 2.f - bounds.left,
            def.y + (def.h - bounds.height) / 2.f - bounds.top - 2.f
        );
        target.draw(label);
    }

    scramblingButtons_.clear();

    if (selectedEncoder_ == "AMI") {
        const float scramY = startY + 3.f * btnH + 2.f * btnGap + 12.f;

        sf::Text scramLabel("AMI Scrambling Scheme:", labelFont(), 24);
        scramLabel.setStyle(sf::Text::Bold);
        scramLabel.setFillColor(sf::Color(71, 85, 105));
        scramLabel.setPosition(cardX + padSide, scramY);
        target.draw(scramLabel);

        const float scramBtnW = (cardWidth - 2.f * padSide - 2.f * 10.f) / 3.f;
        const float scramBtnH = 46.f;
        const float scramBtnY = scramY + 34.f;

        struct ScramDef {
            char code;
            std::string label;
            float x;
        };

        const std::vector<ScramDef> scramDefs = {
            {'N', "None (N)", cardX + padSide},
            {'B', "B8ZS (B)", cardX + padSide + scramBtnW + 10.f},
            {'H', "HDB3 (H)", cardX + padSide + 2.f * (scramBtnW + 10.f)}
        };

        for (const auto& s : scramDefs) {
            const bool isScramSelected = (scramblingChoice_ == s.code);
            scramblingButtons_.push_back({s.code, {s.x, scramBtnY, scramBtnW, scramBtnH}});

            sf::RectangleShape sbtn({scramBtnW, scramBtnH});
            sbtn.setPosition(s.x, scramBtnY);
            sbtn.setFillColor(isScramSelected ? sf::Color(16, 185, 129) : sf::Color(241, 245, 249));
            sbtn.setOutlineThickness(isScramSelected ? 2.5f : 1.5f);
            sbtn.setOutlineColor(isScramSelected ? sf::Color(5, 150, 105) : sf::Color(203, 213, 225));
            target.draw(sbtn);

            sf::Text sText(s.label, labelFont(), 24);
            if (isScramSelected) {
                sText.setStyle(sf::Text::Bold);
                sText.setFillColor(sf::Color(255, 255, 255));
            } else {
                sText.setFillColor(sf::Color(51, 65, 85));
            }

            const sf::FloatRect sb = sText.getLocalBounds();
            sText.setPosition(
                s.x + (scramBtnW - sb.width) / 2.f - sb.left,
                scramBtnY + (scramBtnH - sb.height) / 2.f - sb.top - 2.f
            );
            target.draw(sText);
        }
    } else {
        // Informational properties box for non-AMI encoders
        const float infoY = startY + 3.f * btnH + 2.f * btnGap + 12.f;
        const float infoW = cardWidth - 2.f * padSide;
        const float infoH = 82.f;

        sf::RectangleShape infoBox({infoW, infoH});
        infoBox.setPosition(cardX + padSide, infoY);
        infoBox.setFillColor(sf::Color(248, 250, 252));
        infoBox.setOutlineThickness(1.5f);
        infoBox.setOutlineColor(sf::Color(226, 232, 240));
        target.draw(infoBox);

        sf::Text propTitle("Key Characteristics:", labelFont(), 22);
        propTitle.setStyle(sf::Text::Bold);
        propTitle.setFillColor(sf::Color(71, 85, 105));
        propTitle.setPosition(cardX + padSide + 16.f, infoY + 10.f);
        target.draw(propTitle);

        std::string desc = "Standard line coding waveform.";
        if (selectedEncoder_ == "NRZ-L") {
            desc = "Level-based: Bit 0 -> +1V, Bit 1 -> -1V.";
        } else if (selectedEncoder_ == "NRZ-I") {
            desc = "Inversion on 1: Transition for bit 1, maintain level for 0.";
        } else if (selectedEncoder_ == "Manchester") {
            desc = "Mid-bit transition: Low-to-High = 1, High-to-Low = 0.";
        } else if (selectedEncoder_ == "Differential Manchester") {
            desc = "Mid-bit clock transition. Bit 0 adds transition at interval start.";
        }

        sf::Text propDesc(desc, font_, 20);
        propDesc.setFillColor(sf::Color(100, 116, 139));
        propDesc.setPosition(cardX + padSide + 16.f, infoY + 44.f);
        target.draw(propDesc);
    }
}

void Menu::handleClick(float x, float y)
{
    for (const auto& btn : encoderButtons_) {
        if (btn.second.contains(x, y)) {
            selectedEncoder_ = btn.first;
            return;
        }
    }

    if (selectedEncoder_ == "AMI") {
        for (const auto& sbtn : scramblingButtons_) {
            if (sbtn.second.contains(x, y)) {
                scramblingChoice_ = sbtn.first;
                return;
            }
        }
    }
}

const std::string& Menu::selectedEncoder() const
{
    return selectedEncoder_;
}

void Menu::setSelectedEncoder(const std::string& encoder)
{
    selectedEncoder_ = encoder;
}

char Menu::scramblingChoice() const
{
    return scramblingChoice_;
}

void Menu::setScramblingChoice(char choice)
{
    scramblingChoice_ = choice;
}

} // namespace sigcodec
