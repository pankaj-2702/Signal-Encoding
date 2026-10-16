
#include <SFML/Graphics.hpp>

#include <iostream>
#include <memory>

#include "sigcodec/core/Registry.hpp"

#include "sigcodec/linecode/ILineCoder.hpp"
#include "sigcodec/linecode/NrzL.hpp"
#include "sigcodec/linecode/NrzI.hpp"
#include "sigcodec/linecode/Manchester.hpp"
#include "sigcodec/linecode/DiffManchester.hpp"
#include "sigcodec/linecode/Ami.hpp"

#include "sigcodec/render/SignalPlotter.hpp"
#include "sigcodec/ui/Menu.hpp"
#include "sigcodec/ui/GeneratorPanel.hpp"

using namespace sigcodec;

int main() {
    sf::RenderWindow window(
        sf::VideoMode(1000, 700),
        "Digital Signal Encoding Visualizer"
    );

    window.setFramerateLimit(60);

    // Load font.
    sf::Font font;

    if (!font.loadFromFile(
            "/usr/share/fonts/google-noto/NotoSans-Regular.ttf")) {
        std::cerr << "Could not load font.\n";
        return 1;
    }

    // Register all available line encoders.
    Registry<ILineCoder> registry;

    registry.registerType("NRZ-L", [] {
        return std::make_unique<NrzL>();
    });

    registry.registerType("NRZ-I", [] {
        return std::make_unique<NrzI>();
    });

    registry.registerType("Manchester", [] {
        return std::make_unique<Manchester>();
    });

    registry.registerType("Differential Manchester", [] {
        return std::make_unique<DiffManchester>();
    });

    registry.registerType("AMI", [] {
        return std::make_unique<Ami>();
    });

    // Create UI components.
    Menu menu(font);
    GeneratorPanel generatorPanel(font);
    SignalPlotter plotter;

    while (window.isOpen()) {
        sf::Event event{};

        // Handle window events and user interactions.
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }

            // Handle encoding selector clicks.
            if (event.type == sf::Event::MouseButtonPressed &&
                event.mouseButton.button == sf::Mouse::Left) {

                menu.handleClick(
                    static_cast<float>(event.mouseButton.x),
                    static_cast<float>(event.mouseButton.y)
                );
            }

            // Handle generator controls and text input.
            generatorPanel.handleEvent(event);
        }

        if (!window.isOpen()) {
            break;
        }

        // Read the currently generated bitstream.
        const BitStream& bits = generatorPanel.bits();

        // Create the selected encoder and encode the current bits.
        auto encoder = registry.create(menu.selectedEncoder());
        Signal signal = encoder->encode(bits);

        // Clear the window.
        window.clear(sf::Color(250, 251, 253));

        // Draw encoding selector.
        menu.draw(window);

        // Draw generator controls, bitstream, and longest palindrome.
        generatorPanel.draw(window);

        // Draw the selected encoding waveform.
        plotter.draw(
            window,
            signal,
            bits,
            encoder->name(),
            20.0f,
            285.0f,
            960.0f,
            365.0f,
            sf::Color(35, 110, 210),
            font
        );

        // Display the completed frame.
        window.display();
    }

    return 0;
}
