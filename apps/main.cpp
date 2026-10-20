#include <SFML/Graphics.hpp>

#include <algorithm>
#include <cmath>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "sigcodec/analog/Pcm.hpp"
#include "sigcodec/linecode/ILineCoder.hpp"
#include "sigcodec/linecode/NrzL.hpp"
#include "sigcodec/linecode/NrzI.hpp"
#include "sigcodec/linecode/Manchester.hpp"
#include "sigcodec/linecode/DiffManchester.hpp"
#include "sigcodec/linecode/Ami.hpp"
#include "sigcodec/linecode/B8ZS.hpp"
#include "sigcodec/linecode/HDB3.hpp"

#include "sigcodec/render/SignalPlotter.hpp"
#include "sigcodec/ui/Menu.hpp"
#include "sigcodec/ui/GeneratorPanel.hpp"

#include <iomanip>
#include <sstream>

using namespace sigcodec;

static std::string formatVolt(double v)
{
    if (std::abs(v) < 0.005) {
        v = 0.0;
    }
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(2) << v;
    return ss.str();
}

Signal convertToSignal(const BipolarSignal& pulses)
{
    Signal result;
    result.reserve(pulses.size() * 2);

    for (std::size_t i = 0; i < pulses.size(); ++i) {
        const double start = static_cast<double>(i);
        result.push_back({start, pulses[i]});
        result.push_back({start + 1.0, pulses[i]});
    }

    return result;
}

std::vector<double> generateAnalogSamples(std::size_t count, double amplitude = 1.0)
{
    std::vector<double> samples;
    samples.reserve(count);

    for (std::size_t i = 0; i < count; ++i) {
        const double angle =
            2.0 * 3.14159265358979323846 *
            static_cast<double>(i) / static_cast<double>(count);

        double val = amplitude * std::sin(angle);
        if (std::abs(val) < 1e-12) {
            val = 0.0;
        }
        samples.push_back(val);
    }

    return samples;
}

// Generates a smooth high-resolution continuous sine wave spanning
// [0.0, sampleCount] (one full 2π period) using `resolution` points per
// sample interval — produces a true sine curve instead of a triangle.
Signal generateContinuousSine(std::size_t sampleCount,
                               std::size_t resolution = 360,
                               double amplitude = 1.0)
{
    const double total    = static_cast<double>(sampleCount);
    const std::size_t pts = sampleCount * resolution + 1; // +1 closes at t=8
    const double pi2      = 2.0 * 3.14159265358979323846;

    Signal result;
    result.reserve(pts);

    for (std::size_t i = 0; i < pts; ++i) {
        const double t     = total * static_cast<double>(i)
                             / static_cast<double>(pts - 1);
        const double angle = pi2 * t / total;          // 0 → 2π
        result.push_back({t, amplitude * std::sin(angle)});
    }

    return result;
}


int main()
{
    // Launch on full desktop window
    sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
    if (desktop.width < 1280 || desktop.height < 720) {
        desktop.width = 1920;
        desktop.height = 1080;
    }

    sf::RenderWindow window(
        desktop,
        "Digital Signal Encoding Visualizer",
        sf::Style::Default
    );

    window.setFramerateLimit(60);
    bool isFullscreen = false;

    sf::Font fontRegular;
    if (!fontRegular.loadFromFile("/usr/share/fonts/google-noto/NotoSans-Regular.ttf")) {
        std::cerr << "Could not load regular font.\n";
        return 1;
    }

    sf::Font fontBold;
    const bool hasBold = fontBold.loadFromFile("/usr/share/fonts/google-noto/NotoSans-Bold.ttf");
    const sf::Font* boldFontPtr = hasBold ? &fontBold : nullptr;
    const sf::Font& labelFont = boldFontPtr ? *boldFontPtr : fontRegular;

    Menu menu(fontRegular, boldFontPtr);
    GeneratorPanel generatorPanel(fontRegular, boldFontPtr);
    SignalPlotter plotter;
    Pcm pcm;

    int pcmBitDepth = 4; // Starts in 4-bit PCM mode (16 levels) by default
    double analogAmplitude = 1.0; // Input sine wave amplitude in Volts (0.1V to 1.0V)
    std::vector<double> analogSamples = generateAnalogSamples(8, analogAmplitude);
    BitStream analogBits = pcm.encode(analogSamples, pcmBitDepth);
    PcmResult pcmResult  = pcm.decode(analogBits, pcmBitDepth);
    Signal continuousSine = generateContinuousSine(8, 360, analogAmplitude);

    bool analogMode = false;
    bool signalNeedsUpdate = true;

    auto updateAnalog = [&]() {
        analogSamples = generateAnalogSamples(8, analogAmplitude);
        continuousSine = generateContinuousSine(8, 360, analogAmplitude);
        analogBits = pcm.encode(analogSamples, pcmBitDepth);
        pcmResult = pcm.decode(analogBits, pcmBitDepth);
        signalNeedsUpdate = true;
    };

    BitStream previousBits;
    std::string previousEncoder;
    char previousScrambling = 'N';
    bool previousAnalogMode = false;

    Signal signal;
    std::string signalTitle;

    std::cout
        << "\n=== Signal Encoding Visualizer (Full Window / 3x UI Scale) ===\n"
        << "Controls:\n"
        << "  - Click buttons in the GUI or use keyboard shortcuts:\n"
        << "    [D]: Digital Line Coding Mode\n"
        << "    [A]: Analog PCM Modulation Mode\n"
        << "    [4]: In Analog Mode: 4-bit PCM (16 quantization levels, Default)\n"
        << "    [6]: In Analog Mode: 6-bit PCM (64 quantization levels)\n"
        << "    [+]/[Up]: In Analog Mode: Increase Sine Amplitude (+0.1V)\n"
        << "    [-]/[Down]: In Analog Mode: Decrease Sine Amplitude (-0.1V)\n"
        << "    [N]: AMI Normal (No Scrambling)\n"
        << "    [B]: AMI with B8ZS Scrambling\n"
        << "    [H]: AMI with HDB3 Scrambling\n"
        << "    [F11]: Toggle Fullscreen\n\n";

    while (window.isOpen()) {
        const sf::Vector2u windowSize = window.getSize();
        const float windowWidth = static_cast<float>(windowSize.x);
        const float windowHeight = static_cast<float>(windowSize.y);
        const float margin = 36.f;

        // Scaled mode button bounds on top navigation bar
        const float headerX = margin;
        const float headerW = windowWidth - 2.f * margin;
        const float headerY = 16.f;
        const float headerH = 92.f;
        const float btnW = 280.f;
        const float btnH = 56.f;
        const float btnY = headerY + (headerH - btnH) / 2.f;
        const float digitalBtnX = windowWidth - margin - 24.f - 2.f * btnW - 14.f;
        const float analogBtnX = digitalBtnX + btnW + 14.f;
        const sf::FloatRect digitalBtnBounds(digitalBtnX, btnY, btnW, btnH);
        const sf::FloatRect analogBtnBounds(analogBtnX, btnY, btnW, btnH);

        // Analog Mode control bounds
        const float cardY = 125.f;
        const float cardH = 150.f;
        const float rightEdge = headerX + headerW - 28.f;

        // Row 1: Bit-depth buttons: 4-bit (16 lvls) & 6-bit (64 lvls)
        const float r1BtnW = 210.f;
        const float r1BtnH = 44.f;
        const float r1Y = cardY + 38.f;
        const sf::FloatRect pcm4BtnBounds(rightEdge - 2.f * r1BtnW - 14.f, r1Y, r1BtnW, r1BtnH);
        const sf::FloatRect pcm6BtnBounds(rightEdge - r1BtnW, r1Y, r1BtnW, r1BtnH);

        // Row 2: Amplitude controls: [-] [ Amp: 1.00 V ] [+] [0.50 V] [1.00 V]
        const float r2Y = cardY + 92.f;
        const float r2BtnH = 42.f;
        const float r2StepW = 46.f;
        const float r2ValW = 126.f;
        const float r2PresetW = 88.f;
        const float r2StartX = rightEdge - (r2StepW + 8.f + r2ValW + 8.f + r2StepW + 14.f + r2PresetW + 8.f + r2PresetW);

        const sf::FloatRect ampMinusBtnBounds(r2StartX, r2Y, r2StepW, r2BtnH);
        const sf::FloatRect ampValBoxBounds(r2StartX + r2StepW + 8.f, r2Y, r2ValW, r2BtnH);
        const sf::FloatRect ampPlusBtnBounds(r2StartX + r2StepW + 8.f + r2ValW + 8.f, r2Y, r2StepW, r2BtnH);
        const sf::FloatRect ampPreset1Bounds(r2StartX + r2StepW + 8.f + r2ValW + 8.f + r2StepW + 14.f, r2Y, r2PresetW, r2BtnH); // 0.50 V
        const sf::FloatRect ampPreset2Bounds(rightEdge - r2PresetW, r2Y, r2PresetW, r2BtnH); // 1.00 V

        sf::Event event{};
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }

            if (event.type == sf::Event::Resized) {
                window.setView(sf::View(sf::FloatRect(
                    0.f,
                    0.f,
                    static_cast<float>(event.size.width),
                    static_cast<float>(event.size.height)
                )));
            }

            if (event.type == sf::Event::MouseButtonPressed &&
                event.mouseButton.button == sf::Mouse::Left) {

                const sf::Vector2i pixelPosition(
                    event.mouseButton.x,
                    event.mouseButton.y
                );

                const sf::Vector2f worldPosition =
                    window.mapPixelToCoords(pixelPosition);

                // Check mode buttons
                if (digitalBtnBounds.contains(worldPosition)) {
                    if (analogMode) {
                        analogMode = false;
                        signalNeedsUpdate = true;
                    }
                    continue;
                }

                if (analogBtnBounds.contains(worldPosition)) {
                    if (!analogMode) {
                        analogMode = true;
                        signalNeedsUpdate = true;
                    }
                    continue;
                }

                if (analogMode) {
                    if (pcm4BtnBounds.contains(worldPosition)) {
                        if (pcmBitDepth != 4) {
                            pcmBitDepth = 4;
                            updateAnalog();
                        }
                        continue;
                    }

                    if (pcm6BtnBounds.contains(worldPosition)) {
                        if (pcmBitDepth != 6) {
                            pcmBitDepth = 6;
                            updateAnalog();
                        }
                        continue;
                    }

                    if (ampMinusBtnBounds.contains(worldPosition)) {
                        if (analogAmplitude > 0.15) {
                            analogAmplitude = std::max(0.1, std::round((analogAmplitude - 0.1) * 10.0) / 10.0);
                            updateAnalog();
                        }
                        continue;
                    }

                    if (ampPlusBtnBounds.contains(worldPosition)) {
                        if (analogAmplitude < 0.95) {
                            analogAmplitude = std::min(1.0, std::round((analogAmplitude + 0.1) * 10.0) / 10.0);
                            updateAnalog();
                        }
                        continue;
                    }

                    if (ampPreset1Bounds.contains(worldPosition)) {
                        if (std::abs(analogAmplitude - 0.5) > 0.01) {
                            analogAmplitude = 0.5;
                            updateAnalog();
                        }
                        continue;
                    }

                    if (ampPreset2Bounds.contains(worldPosition)) {
                        if (std::abs(analogAmplitude - 1.0) > 0.01) {
                            analogAmplitude = 1.0;
                            updateAnalog();
                        }
                        continue;
                    }
                } else {
                    menu.handleClick(worldPosition.x, worldPosition.y);

                    sf::Event mappedEvent = event;
                    mappedEvent.mouseButton.x = static_cast<int>(worldPosition.x);
                    mappedEvent.mouseButton.y = static_cast<int>(worldPosition.y);
                    generatorPanel.handleEvent(mappedEvent, windowWidth);
                }

                continue;
            }

            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::F11) {
                    isFullscreen = !isFullscreen;
                    window.create(
                        isFullscreen ? sf::VideoMode::getDesktopMode() : desktop,
                        "Digital Signal Encoding Visualizer",
                        isFullscreen ? sf::Style::Fullscreen : sf::Style::Default
                    );
                    window.setFramerateLimit(60);
                    window.setView(sf::View(sf::FloatRect(
                        0.f,
                        0.f,
                        static_cast<float>(window.getSize().x),
                        static_cast<float>(window.getSize().y)
                    )));
                    continue;
                }

                if (event.key.code == sf::Keyboard::A) {
                    analogMode = true;
                    signalNeedsUpdate = true;
                } else if (event.key.code == sf::Keyboard::D) {
                    analogMode = false;
                    signalNeedsUpdate = true;
                }

                if (analogMode) {
                    if (event.key.code == sf::Keyboard::Num4 || event.key.code == sf::Keyboard::Numpad4) {
                        if (pcmBitDepth != 4) {
                            pcmBitDepth = 4;
                            updateAnalog();
                        }
                    } else if (event.key.code == sf::Keyboard::Num6 || event.key.code == sf::Keyboard::Numpad6) {
                        if (pcmBitDepth != 6) {
                            pcmBitDepth = 6;
                            updateAnalog();
                        }
                    } else if (event.key.code == sf::Keyboard::Up || event.key.code == sf::Keyboard::Add) {
                        if (analogAmplitude < 0.95) {
                            analogAmplitude = std::min(1.0, std::round((analogAmplitude + 0.1) * 10.0) / 10.0);
                            updateAnalog();
                        }
                    } else if (event.key.code == sf::Keyboard::Down || event.key.code == sf::Keyboard::Subtract) {
                        if (analogAmplitude > 0.15) {
                            analogAmplitude = std::max(0.1, std::round((analogAmplitude - 0.1) * 10.0) / 10.0);
                            updateAnalog();
                        }
                    }
                } else if (menu.selectedEncoder() == "AMI") {
                    if (event.key.code == sf::Keyboard::N) {
                        menu.setScramblingChoice('N');
                        signalNeedsUpdate = true;
                    } else if (event.key.code == sf::Keyboard::B) {
                        menu.setScramblingChoice('B');
                        signalNeedsUpdate = true;
                    } else if (event.key.code == sf::Keyboard::H) {
                        menu.setScramblingChoice('H');
                        signalNeedsUpdate = true;
                    }
                }
            }


            if (!analogMode) {
                generatorPanel.handleEvent(event, windowWidth);
            }
        }

        if (!window.isOpen()) {
            break;
        }

        const BitStream& digitalBits = generatorPanel.bits();
        const BitStream& bits = analogMode ? analogBits : digitalBits;
        const std::string selected = menu.selectedEncoder();
        const char scramblingChoice = menu.scramblingChoice();

        if (signalNeedsUpdate ||
            bits != previousBits ||
            selected != previousEncoder ||
            scramblingChoice != previousScrambling ||
            analogMode != previousAnalogMode) {

            std::unique_ptr<ILineCoder> encoder;

            if (selected == "NRZ-L") {
                encoder = std::make_unique<NrzL>();
            } else if (selected == "NRZ-I") {
                encoder = std::make_unique<NrzI>();
            } else if (selected == "Manchester") {
                encoder = std::make_unique<Manchester>();
            } else if (selected == "Differential Manchester" || selected == "Diff Manchester") {
                encoder = std::make_unique<DiffManchester>();
            } else if (selected == "AMI") {
                if (scramblingChoice == 'B') {
                    B8ZS scrambler;
                    signal = convertToSignal(scrambler.encode(bits));
                    signalTitle = "AMI with B8ZS Scrambling";
                } else if (scramblingChoice == 'H') {
                    HDB3 scrambler;
                    signal = convertToSignal(scrambler.encode(bits));
                    signalTitle = "AMI with HDB3 Scrambling";
                } else {
                    encoder = std::make_unique<Ami>();
                }
            }

            if (encoder) {
                signal = encoder->encode(bits);
                signalTitle = encoder->name();

                if (selected == "AMI") {
                    signalTitle = "AMI (No Scrambling)";
                }
            } else if (selected != "AMI") {
                signal.clear();
                signalTitle = "No encoder selected";
            }

            previousBits = bits;
            previousEncoder = selected;
            previousScrambling = scramblingChoice;
            previousAnalogMode = analogMode;
            signalNeedsUpdate = false;
        }

        // Clean light background
        window.clear(sf::Color(245, 247, 250));

        // 1. Top Navigation & Header Bar (3x font and component size)
        sf::RectangleShape headerBox({headerW, headerH});
        headerBox.setPosition(headerX, headerY);
        headerBox.setFillColor(sf::Color(255, 255, 255));
        headerBox.setOutlineThickness(2.0f);
        headerBox.setOutlineColor(sf::Color(226, 232, 240));
        window.draw(headerBox);

        sf::Text appTitle("SIGNAL ENCODING & MODULATION VISUALIZER", labelFont, 36);
        appTitle.setStyle(sf::Text::Bold);
        appTitle.setFillColor(sf::Color(15, 23, 42));
        appTitle.setPosition(headerX + 28.f, headerY + 14.f);
        window.draw(appTitle);

        sf::Text appSub("Line Coding Standards  |  Scrambling Techniques (B8ZS/HDB3)  |  Pulse Code Modulation (PCM)", fontRegular, 20);
        appSub.setFillColor(sf::Color(100, 116, 139));
        appSub.setPosition(headerX + 28.f, headerY + 56.f);
        window.draw(appSub);

        // Mode Switcher Buttons
        auto drawModeBtn = [&](const sf::FloatRect& bounds, const std::string& text, bool isActive) {
            sf::RectangleShape btn({bounds.width, bounds.height});
            btn.setPosition(bounds.left, bounds.top);
            btn.setFillColor(isActive ? sf::Color(37, 99, 235) : sf::Color(241, 245, 249));
            btn.setOutlineThickness(isActive ? 2.5f : 1.5f);
            btn.setOutlineColor(isActive ? sf::Color(29, 78, 216) : sf::Color(203, 213, 225));
            window.draw(btn);

            sf::Text t(text, labelFont, 24);
            if (isActive) {
                t.setStyle(sf::Text::Bold);
                t.setFillColor(sf::Color(255, 255, 255));
            } else {
                t.setFillColor(sf::Color(71, 85, 105));
            }
            const sf::FloatRect tb = t.getLocalBounds();
            t.setPosition(
                bounds.left + (bounds.width - tb.width) / 2.f - tb.left,
                bounds.top + (bounds.height - tb.height) / 2.f - tb.top - 2.f
            );
            window.draw(t);
        };

        drawModeBtn(digitalBtnBounds, "DIGITAL MODE (D)", !analogMode);
        drawModeBtn(analogBtnBounds, "ANALOG PCM (A)", analogMode);

        if (analogMode) {
            // Analog mode information card
            const float cardY = 125.f;
            const float cardH = 150.f;

            sf::RectangleShape infoCard({headerW, cardH});
            infoCard.setPosition(headerX, cardY);
            infoCard.setFillColor(sf::Color(255, 255, 255));
            infoCard.setOutlineThickness(2.0f);
            infoCard.setOutlineColor(sf::Color(226, 232, 240));
            window.draw(infoCard);

            sf::RectangleShape infoAccent({headerW, 5.f});
            infoAccent.setPosition(headerX, cardY);
            infoAccent.setFillColor(sf::Color(16, 185, 129));
            window.draw(infoAccent);

            sf::Text aTitle("ANALOG SIGNAL SAMPLING & PULSE CODE MODULATION (PCM)", labelFont, 28);
            aTitle.setStyle(sf::Text::Bold);
            aTitle.setFillColor(sf::Color(15, 23, 42));
            aTitle.setPosition(headerX + 28.f, cardY + 14.f);
            window.draw(aTitle);

            const int numLevels = 1 << pcmBitDepth;
            const int totalStreamBits = 8 * pcmBitDepth;
            std::string aDescStr = "Input: Sine Wave V(t) = A·sin(2πft), Amplitude: " +
                formatVolt(analogAmplitude) + " V  |  8 samples/cycle  |  Quantization: " +
                std::to_string(pcmBitDepth) + "-bit (" + std::to_string(numLevels) + " levels)  |  Stream: " +
                std::to_string(totalStreamBits) + " bits/cycle";
            sf::Text aDesc(aDescStr, fontRegular, 19);
            aDesc.setFillColor(sf::Color(71, 85, 105));
            aDesc.setPosition(headerX + 28.f, cardY + 54.f);
            window.draw(aDesc);

            sf::Text aHint("Controls: [4] 4-bit, [6] 6-bit  |  [Up]/[+] Increase Amplitude, [Down]/[-] Decrease Amplitude", fontRegular, 17);
            aHint.setFillColor(sf::Color(100, 116, 139));
            aHint.setPosition(headerX + 28.f, cardY + 90.f);
            window.draw(aHint);

            // Row 1 Label: Bit-Depth Selector
            sf::Text r1Hdr("PCM RESOLUTION:", labelFont, 16);
            r1Hdr.setStyle(sf::Text::Bold);
            r1Hdr.setFillColor(sf::Color(100, 116, 139));
            const sf::FloatRect r1Hb = r1Hdr.getLocalBounds();
            r1Hdr.setPosition(pcm4BtnBounds.left - r1Hb.width - 16.f, r1Y + (r1BtnH - r1Hb.height) / 2.f - r1Hb.top - 2.f);
            window.draw(r1Hdr);

            // Row 2 Label: Sine Amplitude
            sf::Text r2Hdr("SINE AMPLITUDE:", labelFont, 16);
            r2Hdr.setStyle(sf::Text::Bold);
            r2Hdr.setFillColor(sf::Color(100, 116, 139));
            const sf::FloatRect r2Hb = r2Hdr.getLocalBounds();
            r2Hdr.setPosition(ampMinusBtnBounds.left - r2Hb.width - 16.f, r2Y + (r2BtnH - r2Hb.height) / 2.f - r2Hb.top - 2.f);
            window.draw(r2Hdr);

            // Generic button drawer
            auto drawPcmBtn = [&](const sf::FloatRect& bounds, const std::string& text, bool isSelected, bool isSpecial = false) {
                sf::RectangleShape btn({bounds.width, bounds.height});
                btn.setPosition(bounds.left, bounds.top);
                if (isSelected) {
                    btn.setFillColor(sf::Color(16, 185, 129));
                    btn.setOutlineThickness(2.5f);
                    btn.setOutlineColor(sf::Color(5, 150, 105));
                } else if (isSpecial) {
                    btn.setFillColor(sf::Color(255, 255, 255));
                    btn.setOutlineThickness(1.8f);
                    btn.setOutlineColor(sf::Color(148, 163, 184));
                } else {
                    btn.setFillColor(sf::Color(241, 245, 249));
                    btn.setOutlineThickness(1.5f);
                    btn.setOutlineColor(sf::Color(203, 213, 225));
                }
                window.draw(btn);

                sf::Text t(text, labelFont, 18);
                t.setStyle(sf::Text::Bold);
                if (isSelected) {
                    t.setFillColor(sf::Color(255, 255, 255));
                } else if (isSpecial) {
                    t.setFillColor(sf::Color(37, 99, 235));
                } else {
                    t.setFillColor(sf::Color(51, 65, 85));
                }
                const sf::FloatRect tb = t.getLocalBounds();
                t.setPosition(
                    bounds.left + (bounds.width - tb.width) / 2.f - tb.left,
                    bounds.top + (bounds.height - tb.height) / 2.f - tb.top - 2.f
                );
                window.draw(t);
            };

            // Row 1 buttons: 4-bit and 6-bit
            drawPcmBtn(pcm4BtnBounds, "4-bit PCM (16 lvls)", pcmBitDepth == 4);
            drawPcmBtn(pcm6BtnBounds, "6-bit PCM (64 lvls)", pcmBitDepth == 6);

            // Row 2 buttons: Amplitude [-], [ Amp: X.XX V ], [+], presets [0.50 V], [1.00 V]
            drawPcmBtn(ampMinusBtnBounds, "-", false);
            drawPcmBtn(ampValBoxBounds, formatVolt(analogAmplitude) + " V", false, true);
            drawPcmBtn(ampPlusBtnBounds, "+", false);
            drawPcmBtn(ampPreset1Bounds, "0.50 V", std::abs(analogAmplitude - 0.5) < 0.01);
            drawPcmBtn(ampPreset2Bounds, "1.00 V", std::abs(analogAmplitude - 1.0) < 0.01);

            const float plotTop = cardY + cardH + 20.f;
            const float plotHeight = std::max(300.f, windowHeight - plotTop - margin);
            const float plotWidth = headerW;

            plotter.drawComparison(
                window,
                continuousSine,           // smooth 2881-pt sine (Blue)
                pcmResult.staircase,      // ZOH staircase reconstruction (Green)
                analogSamples,            // 8 discrete sample dots + stems (Amber)
                analogBits,               // n-bit PCM codewords per column
                pcmBitDepth,              // 4 or 6 bits/sample
                "Original Continuous Sine Wave vs Quantized PCM Reconstruction",
                margin,
                plotTop,
                plotWidth,
                plotHeight,
                fontRegular,
                boldFontPtr
            );
        } else {
            // Digital mode: Control deck (Menu + GeneratorPanel)
            menu.draw(window);
            generatorPanel.draw(window);

            // Centered Signal Waveform Plot
            const float plotTop = 535.f;
            const float plotHeight = std::max(280.f, windowHeight - plotTop - margin);
            const float plotWidth = headerW;

            plotter.draw(
                window,
                signal,
                bits,
                signalTitle,
                margin,
                plotTop,
                plotWidth,
                plotHeight,
                sf::Color(37, 99, 235),
                fontRegular,
                boldFontPtr
            );
        }

        window.display();
    }

    return 0;
}
