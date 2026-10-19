#include "sigcodec/render/SignalPlotter.hpp"

#include <algorithm>
#include <cmath>
#include <string>

namespace sigcodec {

namespace {

void drawThickLine(
    sf::RenderTarget& target,
    sf::Vector2f start,
    sf::Vector2f end,
    const sf::Color& color,
    float thickness = 4.0f)
{
    const float dx = end.x - start.x;
    const float dy = end.y - start.y;
    const float length = std::sqrt(dx * dx + dy * dy);

    if (length < 0.01f) {
        return;
    }

    sf::RectangleShape line({length, thickness});
    line.setOrigin(0.0f, thickness / 2.0f);
    line.setPosition(start);
    line.setRotation(std::atan2(dy, dx) * 180.0f / 3.14159265f);
    line.setFillColor(color);
    target.draw(line);
}

void drawPolyline(
    sf::RenderTarget& target,
    const sf::VertexArray& points,
    const sf::Color& color,
    float thickness = 4.0f)
{
    for (std::size_t i = 1; i < points.getVertexCount(); ++i) {
        drawThickLine(
            target,
            points[i - 1].position,
            points[i].position,
            color,
            thickness
        );
    }
}

} // namespace

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
    const sf::Font& font,
    const sf::Font* boldFont
) const
{
    if (signal.size() < 2 || bits.empty() || width <= 0 || height <= 0) {
        return;
    }

    const sf::Font& labelFont = boldFont ? *boldFont : font;

    // Outer card enclosure
    sf::RectangleShape card({width, height});
    card.setPosition(x, y);
    card.setFillColor(sf::Color(255, 255, 255));
    card.setOutlineThickness(2.0f);
    card.setOutlineColor(sf::Color(226, 232, 240));
    target.draw(card);

    // Top accent line
    sf::RectangleShape topAccent({width, 5.f});
    topAccent.setPosition(x, y);
    topAccent.setFillColor(color);
    target.draw(topAccent);

    // Card Header (3x larger bold font)
    sf::Text heading("WAVEFORM: " + title, labelFont, 36);
    heading.setStyle(sf::Text::Bold);
    heading.setFillColor(sf::Color(15, 23, 42));
    heading.setPosition(x + 28.f, y + 16.f);
    target.draw(heading);

    // Subtitle / Info Badge at top right
    std::string badgeText = std::to_string(bits.size()) + " bits  |  Bipolar Levels: {-1V, 0V, +1V}";
    sf::Text badge(badgeText, font, 24);
    badge.setFillColor(sf::Color(100, 116, 139));
    const sf::FloatRect badgeBounds = badge.getLocalBounds();
    badge.setPosition(x + width - badgeBounds.width - 28.f, y + 24.f);
    target.draw(badge);

    // Centered horizontal padding inside the plot card
    const float padX = 110.0f;
    const float left = x + padX;
    const float right = x + width - padX;
    const float top = y + 105.0f;
    const float bottom = y + height - 70.0f;

    if (right <= left || bottom <= top) {
        return;
    }

    // Inner plot canvas
    sf::RectangleShape innerCanvas({right - left, bottom - top});
    innerCanvas.setPosition(left, top);
    innerCanvas.setFillColor(sf::Color(248, 250, 252));
    innerCanvas.setOutlineThickness(1.5f);
    innerCanvas.setOutlineColor(sf::Color(226, 232, 240));
    target.draw(innerCanvas);

    const double minTime = signal.front().time;
    const double maxTime = signal.back().time;

    if (maxTime <= minTime) {
        return;
    }

    // Determine vertical amplitude scale symmetrically centered on 0.0V
    double maxAbsLevel = 0.0;
    for (const auto& point : signal) {
        maxAbsLevel = std::max(maxAbsLevel, std::abs(point.level));
    }
    if (maxAbsLevel < 1.0) {
        maxAbsLevel = 1.0;
    }

    const double vLimit = maxAbsLevel * 1.38;
    const double minLevel = -vLimit;
    const double maxLevel = +vLimit;

    const auto mapX = [&](double time) {
        return left + static_cast<float>(
            (time - minTime) / (maxTime - minTime) *
            (right - left)
        );
    };

    const auto mapY = [&](double level) {
        return bottom - static_cast<float>(
            (level - minLevel) / (maxLevel - minLevel) *
            (bottom - top)
        );
    };

    // Amplitude axis title (3x larger bold font)
    sf::Text ampTitle("Amplitude (V)", labelFont, 24);
    ampTitle.setStyle(sf::Text::Bold);
    ampTitle.setFillColor(sf::Color(71, 85, 105));
    ampTitle.setPosition(x + 22.f, top - 40.f);
    target.draw(ampTitle);

    // Reference Voltage Grid Lines: +1V, 0V, -1V
    const std::vector<int> levels = {1, 0, -1};
    for (int lvl : levels) {
        const float py = mapY(static_cast<double>(lvl));

        if (lvl == 0) {
            // Prominent horizontal centerline (0V baseline)
            drawThickLine(target, {left, py}, {right, py}, sf::Color(100, 116, 139), 4.0f);

            sf::Text label("0V", labelFont, 26);
            label.setStyle(sf::Text::Bold);
            label.setFillColor(sf::Color(71, 85, 105));
            label.setPosition(left - 58.f, py - 18.f);
            target.draw(label);
        } else {
            // +1V or -1V reference grid lines
            drawThickLine(target, {left, py}, {right, py}, sf::Color(226, 232, 240), 1.5f);

            std::string lvlStr = (lvl > 0 ? "+" : "") + std::to_string(lvl) + "V";
            sf::Text label(lvlStr, labelFont, 26);
            label.setStyle(sf::Text::Bold);
            label.setFillColor(sf::Color(100, 116, 139));
            label.setPosition(left - 70.f, py - 18.f);
            target.draw(label);
        }
    }

    // Bit interval boundaries and centered bit value header
    const double bitDuration = (maxTime - minTime) / static_cast<double>(bits.size());
    const bool showBitCells = (bits.size() <= 36);

    for (std::size_t i = 0; i < bits.size(); ++i) {
        const double startTime = minTime + static_cast<double>(i) * bitDuration;
        const double endTime = startTime + bitDuration;
        const float startX = mapX(startTime);
        const float endX = mapX(endTime);

        // Vertical boundary line
        drawThickLine(target, {startX, top}, {startX, bottom}, sf::Color(226, 232, 240), 1.5f);

        // Bit cell header above the plot
        if (showBitCells) {
            const float cellW = endX - startX;
            sf::RectangleShape bitCell({cellW - 1.f, 38.f});
            bitCell.setPosition(startX, top - 44.f);
            bitCell.setFillColor(bits[i] == 1 ? sf::Color(238, 242, 255) : sf::Color(241, 245, 249));
            bitCell.setOutlineThickness(1.5f);
            bitCell.setOutlineColor(sf::Color(226, 232, 240));
            target.draw(bitCell);

            sf::Text bitText(std::to_string(static_cast<int>(bits[i])), labelFont, 24);
            bitText.setStyle(sf::Text::Bold);
            bitText.setFillColor(bits[i] == 1 ? sf::Color(67, 56, 202) : sf::Color(71, 85, 105));
            const sf::FloatRect btb = bitText.getLocalBounds();
            bitText.setPosition(
                startX + (cellW - btb.width) / 2.f - btb.left,
                top - 44.f + (38.f - btb.height) / 2.f - btb.top - 2.f
            );
            target.draw(bitText);
        }
    }

    // Final boundary line at the right edge
    drawThickLine(target, {mapX(maxTime), top}, {mapX(maxTime), bottom}, sf::Color(226, 232, 240), 1.5f);

    // Waveform polyline (thick 6px stroke)
    sf::VertexArray waveform(sf::LinesStrip, signal.size());
    for (std::size_t i = 0; i < signal.size(); ++i) {
        waveform[i].position = {
            mapX(signal[i].time),
            mapY(signal[i].level)
        };
    }

    drawPolyline(target, waveform, color, 6.0f);

    // Centered Time / Bit period axis label at the bottom (3x larger bold font)
    sf::Text timeLabel("Bit Period (T) / Time  ->", labelFont, 26);
    timeLabel.setStyle(sf::Text::Bold);
    timeLabel.setFillColor(sf::Color(71, 85, 105));
    const sf::FloatRect tlb = timeLabel.getLocalBounds();
    timeLabel.setPosition(
        (left + right - tlb.width) / 2.f - tlb.left,
        bottom + 20.f
    );
    target.draw(timeLabel);
}

void SignalPlotter::drawComparison(
    sf::RenderTarget& target,
    const Signal& originalSignal,
    const Signal& reconstructedSignal,
    const std::string& title,
    float x,
    float y,
    float width,
    float height,
    const sf::Font& font,
    const sf::Font* boldFont
) const
{
    if (originalSignal.size() < 2 || reconstructedSignal.size() < 2 || width <= 0 || height <= 0) {
        return;
    }

    const sf::Font& labelFont = boldFont ? *boldFont : font;

    // Outer card
    sf::RectangleShape card({width, height});
    card.setPosition(x, y);
    card.setFillColor(sf::Color(255, 255, 255));
    card.setOutlineThickness(2.0f);
    card.setOutlineColor(sf::Color(226, 232, 240));
    target.draw(card);

    // Top accent
    sf::RectangleShape topAccent({width, 5.f});
    topAccent.setPosition(x, y);
    topAccent.setFillColor(sf::Color(37, 99, 235));
    target.draw(topAccent);

    // Card Header (3x larger bold font)
    sf::Text heading("ANALOG PCM COMPARISON: " + title, labelFont, 36);
    heading.setStyle(sf::Text::Bold);
    heading.setFillColor(sf::Color(15, 23, 42));
    heading.setPosition(x + 28.f, y + 16.f);
    target.draw(heading);

    // Symmetrical horizontal margins: keeps graph horizontally centered
    const float padX = 110.0f;
    const float left = x + padX;
    const float right = x + width - padX;
    const float top = y + 105.0f;
    const float bottom = y + height - 70.0f;

    if (right <= left || bottom <= top) {
        return;
    }

    // Inner plot canvas
    sf::RectangleShape innerCanvas({right - left, bottom - top});
    innerCanvas.setPosition(left, top);
    innerCanvas.setFillColor(sf::Color(248, 250, 252));
    innerCanvas.setOutlineThickness(1.5f);
    innerCanvas.setOutlineColor(sf::Color(226, 232, 240));
    target.draw(innerCanvas);

    const double minTime = 0.0;
    const double maxTime = static_cast<double>(originalSignal.size() - 1);

    const double minLevel = -1.35;
    const double maxLevel = 1.35;

    const auto mapX = [&](double time) {
        return left + static_cast<float>(
            (time - minTime) / (maxTime - minTime) *
            (right - left)
        );
    };

    const auto mapY = [&](double level) {
        return bottom - static_cast<float>(
            (level - minLevel) / (maxLevel - minLevel) *
            (bottom - top)
        );
    };

    // Horizontal centerline (0V baseline)
    const float py0 = mapY(0.0);
    drawThickLine(target, {left, py0}, {right, py0}, sf::Color(100, 116, 139), 4.0f);

    sf::Text label0("0V", labelFont, 26);
    label0.setStyle(sf::Text::Bold);
    label0.setFillColor(sf::Color(71, 85, 105));
    label0.setPosition(left - 58.f, py0 - 18.f);
    target.draw(label0);

    // +1V and -1V lines
    const float pyP1 = mapY(1.0);
    drawThickLine(target, {left, pyP1}, {right, pyP1}, sf::Color(226, 232, 240), 1.5f);
    sf::Text labelP1("+1V", labelFont, 26);
    labelP1.setStyle(sf::Text::Bold);
    labelP1.setFillColor(sf::Color(100, 116, 139));
    labelP1.setPosition(left - 70.f, pyP1 - 18.f);
    target.draw(labelP1);

    const float pyM1 = mapY(-1.0);
    drawThickLine(target, {left, pyM1}, {right, pyM1}, sf::Color(226, 232, 240), 1.5f);
    sf::Text labelM1("-1V", labelFont, 26);
    labelM1.setStyle(sf::Text::Bold);
    labelM1.setFillColor(sf::Color(100, 116, 139));
    labelM1.setPosition(left - 70.f, pyM1 - 18.f);
    target.draw(labelM1);

    // Vertical sample tick lines
    for (std::size_t i = 0; i < originalSignal.size(); ++i) {
        const float sx = mapX(static_cast<double>(i));
        drawThickLine(target, {sx, top}, {sx, bottom}, sf::Color(226, 232, 240), 1.5f);
    }

    // Original sine wave (Blue, 5.5px thickness)
    sf::VertexArray original(sf::LinesStrip, originalSignal.size());
    for (std::size_t i = 0; i < originalSignal.size(); ++i) {
        original[i].position = {
            mapX(originalSignal[i].time),
            mapY(originalSignal[i].level)
        };
    }
    drawPolyline(target, original, sf::Color(37, 99, 235), 5.5f);

    // PCM staircase reconstruction (Emerald Green, 5.5px thickness)
    sf::VertexArray reconstructed(sf::LinesStrip, reconstructedSignal.size());
    for (std::size_t i = 0; i < reconstructedSignal.size(); ++i) {
        reconstructed[i].position = {
            mapX(reconstructedSignal[i].time),
            mapY(reconstructedSignal[i].level)
        };
    }
    drawPolyline(target, reconstructed, sf::Color(16, 185, 129), 5.5f);

    // Centered Legend at top (3x larger bold font)
    const float legendY = y + 24.0f;

    // Item 1: Original Sine
    sf::RectangleShape key1({32.0f, 6.0f});
    key1.setPosition(x + width - 620.0f, legendY + 12.0f);
    key1.setFillColor(sf::Color(37, 99, 235));
    target.draw(key1);

    sf::Text text1("Original Sine Wave", labelFont, 24);
    text1.setStyle(sf::Text::Bold);
    text1.setFillColor(sf::Color(51, 65, 85));
    text1.setPosition(x + width - 575.0f, legendY);
    target.draw(text1);

    // Item 2: PCM Staircase
    sf::RectangleShape key2({32.0f, 6.0f});
    key2.setPosition(x + width - 300.0f, legendY + 12.0f);
    key2.setFillColor(sf::Color(16, 185, 129));
    target.draw(key2);

    sf::Text text2("PCM Staircase", labelFont, 24);
    text2.setStyle(sf::Text::Bold);
    text2.setFillColor(sf::Color(51, 65, 85));
    text2.setPosition(x + width - 255.0f, legendY);
    target.draw(text2);

    // Centered Time Axis Label
    sf::Text timeLabel("Sample Time (t)  ->", labelFont, 26);
    timeLabel.setStyle(sf::Text::Bold);
    timeLabel.setFillColor(sf::Color(71, 85, 105));
    const sf::FloatRect tlb = timeLabel.getLocalBounds();
    timeLabel.setPosition(
        (left + right - tlb.width) / 2.f - tlb.left,
        bottom + 20.f
    );
    target.draw(timeLabel);
}

} // namespace sigcodec
