#include "sigcodec/render/SignalPlotter.hpp"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>
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

// Simulates a vertical dashed/dotted line by drawing alternating solid segments
void drawDashedVerticalLine(
    sf::RenderTarget& target,
    float x,
    float yStart,
    float yEnd,
    const sf::Color& color,
    float thickness   = 2.0f,
    float dashLen     = 8.0f,
    float gapLen      = 6.0f)
{
    float y = yStart;
    bool drawing = true;
    while (y < yEnd) {
        float segEnd = std::min(y + (drawing ? dashLen : gapLen), yEnd);
        if (drawing) {
            drawThickLine(target, {x, y}, {x, segEnd}, color, thickness);
        }
        y = segEnd;
        drawing = !drawing;
    }
}

// Simulates a horizontal dashed/dotted line by drawing alternating solid segments
void drawDashedHorizontalLine(
    sf::RenderTarget& target,
    float y,
    float xStart,
    float xEnd,
    const sf::Color& color,
    float thickness   = 1.5f,
    float dashLen     = 8.0f,
    float gapLen      = 6.0f)
{
    float x = xStart;
    bool drawing = true;
    while (x < xEnd) {
        float segEnd = std::min(x + (drawing ? dashLen : gapLen), xEnd);
        if (drawing) {
            drawThickLine(target, {x, y}, {segEnd, y}, color, thickness);
        }
        x = segEnd;
        drawing = !drawing;
    }
}


std::string formatVolt(double v)
{
    if (std::abs(v) < 0.005) {
        v = 0.0;
    }
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(2) << v;
    return ss.str();
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

        // Solid boundary line at bit start
        drawThickLine(target, {startX, top}, {startX, bottom}, sf::Color(226, 232, 240), 1.5f);

        // Dotted line at the mid-point of the interval (marks RZ / Manchester transition)
        const float midX = mapX(startTime + bitDuration * 0.5);
        drawDashedVerticalLine(
            target,
            midX, top, bottom,
            sf::Color(148, 163, 184, 200),   // slate-400, semi-transparent
            1.5f,   // thickness
            7.0f,   // dash length
            5.0f    // gap length
        );

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
    const Signal& continuousSignal,
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
    drawComparison(
        target,
        continuousSignal,
        reconstructedSignal,
        std::vector<double>{},
        BitStream{},
        8,       // default bit-depth for the simple overload
        title,
        x,
        y,
        width,
        height,
        font,
        boldFont
    );
}

void SignalPlotter::drawComparison(
    sf::RenderTarget& target,
    const Signal& continuousSignal,
    const Signal& reconstructedSignal,
    const std::vector<double>& discreteSamples,
    const BitStream& pcmBits,
    int bitsPerSample,
    const std::string& title,
    float x,
    float y,
    float width,
    float height,
    const sf::Font& font,
    const sf::Font* boldFont
) const
{
    if (continuousSignal.size() < 2 || reconstructedSignal.size() < 2 || width <= 0 || height <= 0) {
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

    // Top accent (Emerald Green for PCM / Analog)
    sf::RectangleShape topAccent({width, 5.f});
    topAccent.setPosition(x, y);
    topAccent.setFillColor(sf::Color(16, 185, 129));
    target.draw(topAccent);

    // ── Card Header ───────────────────────────────────────────────────────────
    // Row 1: Title (left-aligned)
    sf::Text heading("ANALOG PCM COMPARISON", labelFont, 30);
    heading.setStyle(sf::Text::Bold);
    heading.setFillColor(sf::Color(15, 23, 42));
    heading.setPosition(x + 28.f, y + 12.f);
    target.draw(heading);

    // Row 2: Legend items — centred across the full card width
    //   Approximate widths @ font 20:  "Continuous Sine Wave"~210  "PCM Staircase (ZOH)"~200  "Sample Instants"~160
    //   Total block: 28+8+210 + 44 + 28+8+200 + 44 + 14+8+160 = ~752px  → half = 376
    const float cx  = x + width / 2.f;   // horizontal centre of the card
    const float lY  = y + 48.f;          // legend row Y

    // Item 1: Continuous Sine Wave (Blue)
    sf::RectangleShape lk1({28.f, 6.f});
    lk1.setPosition(cx - 376.f, lY + 12.f);
    lk1.setFillColor(sf::Color(37, 99, 235));
    target.draw(lk1);

    sf::Text lt1("Continuous Sine Wave", labelFont, 20);
    lt1.setStyle(sf::Text::Bold);
    lt1.setFillColor(sf::Color(37, 99, 235));
    lt1.setPosition(cx - 340.f, lY);
    target.draw(lt1);

    // Item 2: PCM Staircase ZOH (Green)
    sf::RectangleShape lk2({28.f, 6.f});
    lk2.setPosition(cx - 70.f, lY + 12.f);
    lk2.setFillColor(sf::Color(16, 185, 129));
    target.draw(lk2);

    sf::Text lt2("PCM Staircase (ZOH)", labelFont, 20);
    lt2.setStyle(sf::Text::Bold);
    lt2.setFillColor(sf::Color(16, 185, 129));
    lt2.setPosition(cx - 34.f, lY);
    target.draw(lt2);

    // Item 3: Sample Instants (Amber)
    sf::CircleShape lk3(7.f);
    lk3.setOrigin(7.f, 7.f);
    lk3.setPosition(cx + 218.f, lY + 15.f);
    lk3.setFillColor(sf::Color(245, 158, 11));
    lk3.setOutlineThickness(2.f);
    lk3.setOutlineColor(sf::Color(255, 255, 255));
    target.draw(lk3);

    sf::Text lt3("Sample Instants", labelFont, 20);
    lt3.setStyle(sf::Text::Bold);
    lt3.setFillColor(sf::Color(245, 158, 11));
    lt3.setPosition(cx + 234.f, lY);
    target.draw(lt3);

    // Row 3: Subtitle — dynamically reflects the active bit depth
    const int numLevels = 1 << bitsPerSample;   // 2^n
    const int totalBits = static_cast<int>(discreteSamples.size()) * bitsPerSample;
    std::string specStr =
        "8 samples/cycle  |  " +
        std::to_string(bitsPerSample) + "-bit PCM  |  " +
        std::to_string(numLevels) + " quantization levels  |  " +
        std::to_string(totalBits) + " bits/cycle";
    sf::Text subHeading(specStr, font, 19);
    subHeading.setFillColor(sf::Color(100, 116, 139));
    subHeading.setPosition(x + 28.f, y + 80.f);
    target.draw(subHeading);

    // ── Plot canvas dimensions ────────────────────────────────────────────────
    // padX=140 gives 140px left margin for "Amplitude" rotated label + ±1V/0V labels
    const float padX  = 140.0f;
    const float left  = x + padX;
    const float right = x + width - 110.f;  // 110px right margin for quantization level labels
    // top: title(38) + legend(30) + subtitle(26) + cell-header(70) + gap(16) = ~190
    const float top    = y + 190.0f;
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

    // Full duration spans from 0.0 to 8.0 (1 full period)
    const double minTime = 0.0;
    double maxTime = 8.0;
    if (!reconstructedSignal.empty()) {
        maxTime = std::max(maxTime, reconstructedSignal.back().time);
    }
    if (!continuousSignal.empty()) {
        maxTime = std::max(maxTime, continuousSignal.back().time);
    }

    // Symmetrical vertical voltage limits centered exactly around 0.0V
    const double minLevel = -1.38;
    const double maxLevel = 1.38;

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
    {   // right-align against the canvas left edge
        const sf::FloatRect b = label0.getLocalBounds();
        label0.setPosition(left - b.width - b.left - 10.f, py0 - b.height / 2.f - b.top);
    }
    target.draw(label0);

    // +1V and -1V lines
    const float pyP1 = mapY(1.0);
    drawThickLine(target, {left, pyP1}, {right, pyP1}, sf::Color(226, 232, 240), 1.5f);
    sf::Text labelP1("+1V", labelFont, 26);
    labelP1.setStyle(sf::Text::Bold);
    labelP1.setFillColor(sf::Color(100, 116, 139));
    {
        const sf::FloatRect b = labelP1.getLocalBounds();
        labelP1.setPosition(left - b.width - b.left - 10.f, pyP1 - b.height / 2.f - b.top);
    }
    target.draw(labelP1);

    const float pyM1 = mapY(-1.0);
    drawThickLine(target, {left, pyM1}, {right, pyM1}, sf::Color(226, 232, 240), 1.5f);
    sf::Text labelM1("-1V", labelFont, 26);
    labelM1.setStyle(sf::Text::Bold);
    labelM1.setFillColor(sf::Color(100, 116, 139));
    {
        const sf::FloatRect b = labelM1.getLocalBounds();
        labelM1.setPosition(left - b.width - b.left - 10.f, pyM1 - b.height / 2.f - b.top);
    }
    target.draw(labelM1);


    // ── Quantization Levels & Decision Boundaries ─────────────────────────────
    // For n-bit PCM, there are numLevels = 2^n quantization codes (0 to maxCode).
    // The quantizer maps [-1V, +1V] to discrete reconstruction levels:
    //   vLevel(k) = -1.0 + 2.0 * k / maxCode, for k = 0 … maxCode
    // The decision boundaries between adjacent levels are located at midpoints:
    //   vBound(k) = -1.0 + 2.0 * (k + 0.5) / maxCode, for k = 0 … maxCode-1
    // For 6-bit PCM (64 levels), lines and tags are omitted to avoid dense visual overlap.
    if (numLevels <= 16) {
        const int maxCode = std::max(1, numLevels - 1);

        // 1. Draw Decision Boundaries (Dashed horizontal lines separating quantization regions)
        for (int k = 0; k < maxCode; ++k) {
            const double vBound = -1.0 + (2.0 * (static_cast<double>(k) + 0.5)) / static_cast<double>(maxCode);
            if (std::abs(vBound) < 1e-4) {
                // Skip exact 0V since prominent centerline is already there
                continue;
            }
            const float py = mapY(vBound);
            drawDashedHorizontalLine(target, py, left, right, sf::Color(148, 163, 184, 150), 1.5f, 6.0f, 6.0f);
        }

        // 2. Draw Reconstruction Levels (Solid lines + Right-side labels)
        for (int k = 0; k < numLevels; ++k) {
            const double vLevel = -1.0 + (2.0 * static_cast<double>(k)) / static_cast<double>(maxCode);
            const float py = mapY(vLevel);

            const bool isLimit = (k == 0 || k == maxCode);
            const bool isCenter = (std::abs(vLevel) < 0.005);

            // Draw interior level line
            if (!isLimit && !isCenter) {
                drawThickLine(target, {left, py}, {right, py}, sf::Color(16, 185, 129, 120), 1.2f);
            }

            // Generate binary codeword string for this level
            std::string codeStr;
            for (int b = bitsPerSample - 1; b >= 0; --b) {
                codeStr += ((k >> b) & 1) ? '1' : '0';
            }

            // Right-side level indicator label
            std::string lvlTag;
            if (numLevels <= 4) {
                lvlTag = "Q" + std::to_string(k) + ": " + (vLevel >= 0.005 ? "+" : "") + formatVolt(vLevel) + "V [" + codeStr + "]";
            } else {
                lvlTag = "Q" + std::to_string(k) + " [" + codeStr + "]";
            }

            sf::Text qText(lvlTag, font, (numLevels <= 4 ? 16 : 13));
            qText.setStyle(sf::Text::Bold);
            qText.setFillColor(isLimit || isCenter ? sf::Color(15, 23, 42) : sf::Color(5, 150, 105));
            const sf::FloatRect qb = qText.getLocalBounds();
            qText.setPosition(right + 8.f, py - qb.height / 2.f - qb.top);
            target.draw(qText);
        }
    }


    // Amplitude axis title — rotated vertically, centred on the left margin
    sf::Text ampTitle("Amplitude (V)", labelFont, 20);
    ampTitle.setStyle(sf::Text::Bold);
    ampTitle.setFillColor(sf::Color(71, 85, 105));
    {
        const sf::FloatRect ab = ampTitle.getLocalBounds();
        ampTitle.setOrigin(ab.left + ab.width / 2.f, ab.top + ab.height / 2.f);
        ampTitle.setRotation(-90.f);
        ampTitle.setPosition(x + 18.f, (top + bottom) / 2.f);
    }
    target.draw(ampTitle);

    // Vertical sample tick lines and bottom time tick marks (0, Ts, 2Ts, ... 8Ts)
    const int numIntervals = static_cast<int>(std::round(maxTime));
    for (int i = 0; i <= numIntervals; ++i) {
        const float sx = mapX(static_cast<double>(i));
        drawThickLine(target, {sx, top}, {sx, bottom}, sf::Color(226, 232, 240), 1.5f);

        std::string tickStr = (i == 0) ? "0" : (i == numIntervals ? std::to_string(i) + "Ts (T)" : std::to_string(i) + "Ts");
        sf::Text tickLabel(tickStr, labelFont, 20);
        tickLabel.setStyle(sf::Text::Bold);
        tickLabel.setFillColor(sf::Color(100, 116, 139));
        const sf::FloatRect tb = tickLabel.getLocalBounds();
        tickLabel.setPosition(sx - tb.width / 2.f - tb.left, bottom + 6.f);
        target.draw(tickLabel);
    }

    // Top Sample Header Band & Discrete Sample Markers (Stems & Dots)
    if (!discreteSamples.empty()) {
        const std::size_t sampleCount = discreteSamples.size();
        for (std::size_t k = 0; k < sampleCount; ++k) {
            const double t = static_cast<double>(k);
            const double val = discreteSamples[k];
            const float sx = mapX(t);
            const float sy = mapY(val);

            // Vertical stem from 0V baseline to sample point on sine curve
            drawThickLine(target, {sx, py0}, {sx, sy}, sf::Color(245, 158, 11, 190), 2.5f);

            // Circular marker dot at the sample point
            sf::CircleShape dot(7.5f);
            dot.setOrigin(7.5f, 7.5f);
            dot.setPosition(sx, sy);
            dot.setFillColor(sf::Color(245, 158, 11)); // Amber gold
            dot.setOutlineThickness(2.5f);
            dot.setOutlineColor(sf::Color(255, 255, 255));
            target.draw(dot);

            // Header cell above the graph — sits just above the plot canvas
            const float nextSx = mapX(t + 1.0);
            const float colW   = nextSx - sx;
            const float cellH  = 64.f;
            const float cellTop = top - cellH - 6.f;  // 6px gap above canvas

            sf::RectangleShape cellBox({colW - 2.f, cellH});
            cellBox.setPosition(sx, cellTop);
            cellBox.setFillColor(k % 2 == 0 ? sf::Color(238, 242, 255) : sf::Color(241, 245, 249));
            cellBox.setOutlineThickness(1.5f);
            cellBox.setOutlineColor(sf::Color(203, 213, 225));
            target.draw(cellBox);

            // Sample name + voltage (top half of cell)
            std::string sText = "S" + std::to_string(k) + ": " +
                                (val >= 0.0 ? "+" : "") + formatVolt(val) + "V";
            sf::Text sLabel(sText, labelFont, 18);
            sLabel.setStyle(sf::Text::Bold);
            sLabel.setFillColor(sf::Color(15, 23, 42));
            const sf::FloatRect slb = sLabel.getLocalBounds();
            sLabel.setPosition(
                sx + (colW - slb.width) / 2.f - slb.left,
                cellTop + 4.f
            );
            target.draw(sLabel);

            // PCM n-bit codeword (bottom half of cell)
            const std::size_t bps = static_cast<std::size_t>(bitsPerSample > 0 ? bitsPerSample : 4);
            if (pcmBits.size() >= (k + 1) * bps) {
                std::string codeStr;
                for (std::size_t b = 0; b < bps; ++b) {
                    codeStr += (pcmBits[k * bps + b] == 0 ? '0' : '1');
                }

                const unsigned int cSize = (bps >= 6 ? 19 : 22);
                sf::Text cLabel(codeStr, font, cSize);
                cLabel.setStyle(sf::Text::Bold);
                cLabel.setFillColor(sf::Color(37, 99, 235));
                const sf::FloatRect clb = cLabel.getLocalBounds();
                cLabel.setPosition(
                    sx + (colW - clb.width) / 2.f - clb.left,
                    cellTop + 34.f
                );
                target.draw(cLabel);
            }

        }
    }

    // 1. Continuous smooth sine wave (Blue, thick 6.0px stroke)
    sf::VertexArray continuous(sf::LinesStrip, continuousSignal.size());
    for (std::size_t i = 0; i < continuousSignal.size(); ++i) {
        continuous[i].position = {
            mapX(continuousSignal[i].time),
            mapY(continuousSignal[i].level)
        };
    }
    drawPolyline(target, continuous, sf::Color(37, 99, 235), 6.0f);

    // 2. Reconstructed PCM staircase (Emerald Green, 5.0px stroke)
    sf::VertexArray reconstructed(sf::LinesStrip, reconstructedSignal.size());
    for (std::size_t i = 0; i < reconstructedSignal.size(); ++i) {
        reconstructed[i].position = {
            mapX(reconstructedSignal[i].time),
            mapY(reconstructedSignal[i].level)
        };
    }
    drawPolyline(target, reconstructed, sf::Color(16, 185, 129), 5.0f);

    // Centered Time Axis Label
    sf::Text timeLabel("Sampling Period (t = n \xc2\xb7 Ts)  ->", labelFont, 24);
    timeLabel.setStyle(sf::Text::Bold);
    timeLabel.setFillColor(sf::Color(71, 85, 105));
    const sf::FloatRect tlb = timeLabel.getLocalBounds();
    timeLabel.setPosition(
        (left + right - tlb.width) / 2.f - tlb.left,
        bottom + 28.f
    );
    target.draw(timeLabel);
}

} // namespace sigcodec
