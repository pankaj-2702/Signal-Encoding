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

using namespace sigcodec;

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

std::vector<double> generateAnalogSamples(std::size_t count)
{
    std::vector<double> samples;
    samples.reserve(count);

    for (std::size_t i = 0; i < count; ++i) {
        const double angle =
            2.0 * 3.14159265358979323846 *
            static_cast<double>(i) / static_cast<double>(count);

        samples.push_back(std::sin(angle));
    }

    return samples;
}

Signal samplesToSignal(const std::vector<double>& samples)
{
    Signal result;
    result.reserve(samples.size());

    for (std::size_t i = 0; i < samples.size(); ++i) {
        result.push_back({
            static_cast<double>(i),
            samples[i]
        });
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

    const std::vector<double> analogSamples = generateAnalogSamples(8);
    const BitStream analogBits = pcm.encode(analogSamples, 8);
    const PcmResult pcmResult = pcm.decode(analogBits, 8);
    const Signal originalSignal = samplesToSignal(analogSamples);

    bool analogMode = false;
    bool signalNeedsUpdate = true;

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
        const float headerY = 16.f;
        const float headerH = 92.f;
        const float btnW = 280.f;
        const float btnH = 56.f;
        const float btnY = headerY + (headerH - btnH) / 2.f;
        const float digitalBtnX = windowWidth - margin - 24.f - 2.f * btnW - 14.f;
        const float analogBtnX = digitalBtnX + btnW + 14.f;
        const sf::FloatRect digitalBtnBounds(digitalBtnX, btnY, btnW, btnH);
        const sf::FloatRect analogBtnBounds(analogBtnX, btnY, btnW, btnH);

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

                if (!analogMode) {
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

                if (!analogMode && menu.selectedEncoder() == "AMI") {
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
        const float headerX = margin;
        const float headerW = windowWidth - 2.f * margin;

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
            const float cardH = 135.f;

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

            sf::Text aTitle("ANALOG SIGNAL SAMPLING & PULSE CODE MODULATION (PCM)", labelFont, 32);
            aTitle.setStyle(sf::Text::Bold);
            aTitle.setFillColor(sf::Color(15, 23, 42));
            aTitle.setPosition(headerX + 28.f, cardY + 18.f);
            window.draw(aTitle);

            sf::Text aDesc("Input: Continuous Sine Wave | Sampling Rate: 8 samples/cycle | Quantization: 8 bits/sample | Total Stream: 64 bits", fontRegular, 22);
            aDesc.setFillColor(sf::Color(71, 85, 105));
            aDesc.setPosition(headerX + 28.f, cardY + 58.f);
            window.draw(aDesc);

            sf::Text aHint("Press 'D' or click [DIGITAL MODE] to return to line encoding schemes.", fontRegular, 20);
            aHint.setFillColor(sf::Color(100, 116, 139));
            aHint.setPosition(headerX + 28.f, cardY + 92.f);
            window.draw(aHint);

            const float plotTop = cardY + cardH + 24.f;
            const float plotHeight = std::max(300.f, windowHeight - plotTop - margin);
            const float plotWidth = headerW;

            plotter.drawComparison(
                window,
                originalSignal,
                pcmResult.staircase,
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
