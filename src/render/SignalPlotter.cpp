#include "sigcodec/render/SignalPlotter.hpp"

#include <algorithm>
#include <cmath>
#include <sstream>

namespace sigcodec {

void SignalPlotter::draw(
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
) const
{
    if (signal.size() < 2 || bits.empty() ||
        width <= 0 || height <= 0) {
        return;
    }

    const double minTime = signal.front().time;
    const double maxTime = signal.back().time;

    if (maxTime <= minTime) {
        return;
    }

    double minLevel = signal.front().level;
    double maxLevel = signal.front().level;

    for (const auto& point : signal) {
        minLevel = std::min(minLevel, point.level);
        maxLevel = std::max(maxLevel, point.level);
    }

    // Include zero in the visible range.
    minLevel = std::min(minLevel, 0.0);
    maxLevel = std::max(maxLevel, 0.0);

    if (minLevel == maxLevel) {
        minLevel -= 1.0;
        maxLevel += 1.0;
    }

    // Leave room for labels on the left and right.
    const float left = x + 55.0f;
    const float right = x + width - 15.0f;
    const float top = y + 35.0f;
    const float bottom = y + height - 35.0f;

    const auto mapX = [&](double time) {
        return left + static_cast<float>(
            (time - minTime) / (maxTime - minTime)
            * (right - left)
        );
    };

    const auto mapY = [&](double level) {
        return bottom - static_cast<float>(
            (level - minLevel) / (maxLevel - minLevel)
            * (bottom - top)
        );
    };

    // Encoder title.
    sf::Text heading(title, font, 18);
    heading.setFillColor(sf::Color(35, 35, 35));
    heading.setPosition(x + 5.0f, y);
    target.draw(heading);

    // Horizontal voltage grid and labels.
    for (int level = static_cast<int>(std::ceil(minLevel));
         level <= static_cast<int>(std::floor(maxLevel));
         ++level) {

        const float py = mapY(level);

        sf::Vertex grid[] = {
            sf::Vertex({left, py}, sf::Color(225, 225, 225)),
            sf::Vertex({right, py}, sf::Color(225, 225, 225))
        };

        target.draw(grid, 2, sf::Lines);

        sf::Text label(
            std::to_string(level),
            font,
            13
        );

        label.setFillColor(sf::Color(80, 80, 80));
        label.setPosition(x + 15.0f, py - 9.0f);
        target.draw(label);
    }

    // Draw the waveform using the actual encoded points.
    sf::VertexArray waveform(sf::LinesStrip, signal.size());

    for (std::size_t i = 0; i < signal.size(); ++i) {
        waveform[i].position = {
            mapX(signal[i].time),
            mapY(signal[i].level)
        };

        waveform[i].color = color;
    }

    target.draw(waveform);

    // Bit labels and time ticks at bit boundaries.
    const double bitDuration =
        (maxTime - minTime) / static_cast<double>(bits.size());

    for (std::size_t i = 0; i < bits.size(); ++i) {
        const double startTime =
            minTime + i * bitDuration;

        const double endTime =
            startTime + bitDuration;

        const float centerX =
            (mapX(startTime) + mapX(endTime)) / 2.0f;

        sf::Text bitLabel(
            std::to_string(static_cast<int>(bits[i])),
            font,
            14
        );

        bitLabel.setFillColor(sf::Color::Black);
        bitLabel.setOrigin(
            bitLabel.getLocalBounds().left
                + bitLabel.getLocalBounds().width / 2.0f,
            0.0f
        );

        bitLabel.setPosition(centerX, bottom + 8.0f);
        target.draw(bitLabel);

        sf::Vertex tick[] = {
            sf::Vertex(
                {mapX(startTime), top},
                sf::Color(205, 205, 205)
            ),
            sf::Vertex(
                {mapX(startTime), bottom},
                sf::Color(205, 205, 205)
            )
        };

        target.draw(tick, 2, sf::Lines);
    }

    // Final bit boundary.
    sf::Vertex finalTick[] = {
        sf::Vertex(
            {mapX(maxTime), top},
            sf::Color(205, 205, 205)
        ),
        sf::Vertex(
            {mapX(maxTime), bottom},
            sf::Color(205, 205, 205)
        )
    };

    target.draw(finalTick, 2, sf::Lines);

    // Time-axis label.
    sf::Text timeLabel("Time", font, 13);
    timeLabel.setFillColor(sf::Color(70, 70, 70));
    timeLabel.setPosition(right - 30.0f, bottom + 24.0f);
    target.draw(timeLabel);
}

} // namespace sigcodec