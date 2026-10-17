
#include <SFML/Graphics.hpp>

#include <iostream>
#include <memory>
#include <string>

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

// Convert pulse levels into points that the waveform plotter can draw.
Signal convertToSignal(const BipolarSignal& pulses)
{
    Signal signal;
    signal.reserve(pulses.size() * 2);

    for (std::size_t i = 0; i < pulses.size(); ++i) {
        double startTime = static_cast<double>(i);
        double endTime = startTime + 1.0;

        signal.push_back({startTime, pulses[i]});
        signal.push_back({endTime, pulses[i]});
    }

    return signal;
}

int main()
{
    sf::RenderWindow window(
        sf::VideoMode(1000, 700),
        "Digital Signal Encoding Visualizer"
    );

    window.setFramerateLimit(60);

    sf::Font font;

    if (!font.loadFromFile(
            "/usr/share/fonts/google-noto/NotoSans-Regular.ttf")) {
        std::cerr << "Could not load font.\n";
        return 1;
    }

    Menu menu(font);
    GeneratorPanel generatorPanel(font);
    SignalPlotter plotter;

    BitStream previousBits;
    std::string previousEncoder;
    char previousScrambling = 'N';

    Signal signal;
    std::string signalTitle;

    // N = normal AMI, B = B8ZS, H = HDB3.
    char scramblingChoice = 'N';

    std::cout << "\nAMI scrambling controls:\n";
    std::cout << "N: Normal AMI (no scrambling)\n";
    std::cout << "B: B8ZS scrambling\n";
    std::cout << "H: HDB3 scrambling\n";
    std::cout << "These keys apply when AMI is selected.\n\n";

    bool signalNeedsUpdate = true;

    while (window.isOpen()) {
        sf::Event event{};

        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }

            if (event.type == sf::Event::MouseButtonPressed &&
                event.mouseButton.button == sf::Mouse::Left) {

                menu.handleClick(
                    static_cast<float>(event.mouseButton.x),
                    static_cast<float>(event.mouseButton.y)
                );
            }

            // Handle keyboard choices for AMI scrambling.
            if (event.type == sf::Event::KeyPressed &&
                menu.selectedEncoder() == "AMI") {

                if (event.key.code == sf::Keyboard::N) {
                    scramblingChoice = 'N';
                    signalNeedsUpdate = true;
                }
                else if (event.key.code == sf::Keyboard::B) {
                    scramblingChoice = 'B';
                    signalNeedsUpdate = true;
                }
                else if (event.key.code == sf::Keyboard::H) {
                    scramblingChoice = 'H';
                    signalNeedsUpdate = true;
                }
            }

            generatorPanel.handleEvent(event);
        }

        if (!window.isOpen()) {
            break;
        }

        const BitStream& bits = generatorPanel.bits();
        const std::string selected = menu.selectedEncoder();

        if (signalNeedsUpdate ||
            bits != previousBits ||
            selected != previousEncoder ||
            scramblingChoice != previousScrambling) {

            std::unique_ptr<ILineCoder> encoder;

            if (selected == "NRZ-L") {
                encoder = std::make_unique<NrzL>();
            }
            else if (selected == "NRZ-I") {
                encoder = std::make_unique<NrzI>();
            }
            else if (selected == "Manchester") {
                encoder = std::make_unique<Manchester>();
            }
            else if (selected == "Differential Manchester") {
                encoder = std::make_unique<DiffManchester>();
            }
            else if (selected == "AMI") {
                if (scramblingChoice == 'B') {
                    B8ZS scrambler;
                    signal = convertToSignal(scrambler.encode(bits));
                    signalTitle = "AMI with B8ZS";
                }
                else if (scramblingChoice == 'H') {
                    HDB3 scrambler;
                    signal = convertToSignal(scrambler.encode(bits));
                    signalTitle = "AMI with HDB3";
                }
                else {
                    encoder = std::make_unique<Ami>();
                }
            }

            // Encode normally for the non-scrambled techniques and AMI.
            if (encoder) {
                signal = encoder->encode(bits);
                signalTitle = encoder->name();

                if (selected == "AMI") {
                    signalTitle = "AMI (no scrambling)";
                }
            }

            if (!encoder && selected != "AMI") {
                signal.clear();
                signalTitle = "No encoder selected";
            }

            previousBits = bits;
            previousEncoder = selected;
            previousScrambling = scramblingChoice;
            signalNeedsUpdate = false;
        }

        window.clear(sf::Color(250, 251, 253));

        menu.draw(window);
        generatorPanel.draw(window);

        plotter.draw(
            window,
            signal,
            bits,
            signalTitle,
            20.0f,
            285.0f,
            960.0f,
            365.0f,
            sf::Color(35, 110, 210),
            font
        );

        window.display();
    }

    return 0;
}
