#include "sigcodec/analog/Pcm.hpp"

#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>

int main() {
    sigcodec::Pcm pcm;

    // ─────────────────────────────────────────────────────────────────────────
    // Test 1: 4-bit PCM Mode (Default - 16 Quantization Levels)
    // ─────────────────────────────────────────────────────────────────────────
    {
        std::vector<double> samples = {-1.0, -0.5, 0.0, 0.5, 1.0};
        const int bitsPerSample = 4;
        auto bits = pcm.encode(samples, bitsPerSample);
        auto decoded = pcm.decode(bits, bitsPerSample);

        // Verify bits per sample and total stream size: 5 samples * 4 bits = 20 bits
        assert(bits.size() == 20);
        assert(decoded.reconstructedSamples.size() == 5);
        assert(decoded.staircase.size() == 10);

        // Boundary -1.0V -> Code 0 (0000) -> Reconstructed -1.0V
        assert(bits[0] == 0 && bits[1] == 0 && bits[2] == 0 && bits[3] == 0);
        assert(std::abs(decoded.reconstructedSamples[0] - (-1.0)) < 1e-4);

        // Boundary +1.0V -> Code 15 (1111) -> Reconstructed +1.0V
        assert(bits[16] == 1 && bits[17] == 1 && bits[18] == 1 && bits[19] == 1);
        assert(std::abs(decoded.reconstructedSamples[4] - (1.0)) < 1e-4);

        // Center 0.0V -> normalized 0.5 -> round(0.5 * 15) = 8 (1000)
        assert(bits[8] == 1 && bits[9] == 0 && bits[10] == 0 && bits[11] == 0);

        // Test boundary clamping with values outside [-1.0, 1.0]
        std::vector<double> outOfBounds = {-1.5, 1.5};
        auto clampedBits = pcm.encode(outOfBounds, bitsPerSample);
        assert(clampedBits[0] == 0 && clampedBits[1] == 0 && clampedBits[2] == 0 && clampedBits[3] == 0);
        assert(clampedBits[4] == 1 && clampedBits[5] == 1 && clampedBits[6] == 1 && clampedBits[7] == 1);

        std::cout << "PASS: 4-bit PCM tests (16 levels, 4 bits/sample)\n";
    }

    // ─────────────────────────────────────────────────────────────────────────
    // Test 2: 6-bit PCM Mode (64 Quantization Levels)
    // ─────────────────────────────────────────────────────────────────────────
    {
        std::vector<double> samples = {-1.0, -0.5, 0.0, 0.5, 1.0};
        const int bitsPerSample = 6;
        auto bits = pcm.encode(samples, bitsPerSample);
        auto decoded = pcm.decode(bits, bitsPerSample);

        // Verify bits per sample and total stream size: 5 samples * 6 bits = 30 bits
        assert(bits.size() == 30);
        assert(decoded.reconstructedSamples.size() == 5);
        assert(decoded.staircase.size() == 10);

        // Code 0: -1.0V -> binary 000000 -> -1.00V
        for (int b = 0; b < 6; ++b) {
            assert(bits[b] == 0);
        }
        assert(std::abs(decoded.reconstructedSamples[0] - (-1.0)) < 1e-4);

        // Code 63: +1.0V -> binary 111111 -> +1.00V
        for (int b = 24; b < 30; ++b) {
            assert(bits[b] == 1);
        }
        assert(std::abs(decoded.reconstructedSamples[4] - (1.0)) < 1e-4);

        // Center 0.0V -> normalized 0.5 -> round(0.5 * 63) = 32 (100000)
        assert(bits[12] == 1 && bits[13] == 0 && bits[14] == 0 &&
               bits[15] == 0 && bits[16] == 0 && bits[17] == 0);

        // Intermediate -0.5V -> normalized 0.25 -> round(0.25 * 63) = 16 (010000)
        assert(bits[6] == 0 && bits[7] == 1 && bits[8] == 0 &&
               bits[9] == 0 && bits[10] == 0 && bits[11] == 0);

        // Test boundary clamping with values outside [-1.0, 1.0]
        std::vector<double> outOfBounds = {-2.0, 2.0};
        auto clampedBits = pcm.encode(outOfBounds, bitsPerSample);
        for (int b = 0; b < 6; ++b) {
            assert(clampedBits[b] == 0);
            assert(clampedBits[6 + b] == 1);
        }

        std::cout << "PASS: 6-bit PCM tests (64 levels, 6 bits/sample)\n";
    }

    // ─────────────────────────────────────────────────────────────────────────
    // Test 3: Amplitude Scaling (Dynamic Range Demonstration)
    // ─────────────────────────────────────────────────────────────────────────
    {
        // Amplitude = 0.5V vs 1.0V
        const double amp = 0.5;
        std::vector<double> scaledSamples = {-amp, 0.0, amp};

        // In 4-bit mode: -0.5V maps to code 4 (0100) instead of code 0 (0000)
        auto bits4 = pcm.encode(scaledSamples, 4);
        assert(bits4[0] == 0 && bits4[1] == 1 && bits4[2] == 0 && bits4[3] == 0);

        // In 6-bit mode: -0.5V maps to code 16 (010000) instead of code 0 (000000)
        auto bits6 = pcm.encode(scaledSamples, 6);
        assert(bits6[0] == 0 && bits6[1] == 1 && bits6[2] == 0 &&
               bits6[3] == 0 && bits6[4] == 0 && bits6[5] == 0);

        std::cout << "PASS: Amplitude scaling tests (0.5V vs 1.0V)\n";
    }

    // ─────────────────────────────────────────────────────────────────────────
    // Test 4: Bit-string conversion & error handling
    // ─────────────────────────────────────────────────────────────────────────
    {
        assert(sigcodec::Pcm::bitsToString({1, 0, 1, 1}) == "1011");
        assert(sigcodec::Pcm::bitsToString({1, 0, 1, 1, 0, 0}) == "101100");

        // Verify invalid bits per sample throws std::invalid_argument
        bool threwLow = false;
        try {
            pcm.encode({0.0}, 0);
        } catch (const std::invalid_argument&) {
            threwLow = true;
        }
        assert(threwLow);

        bool threwHigh = false;
        try {
            pcm.encode({0.0}, 17);
        } catch (const std::invalid_argument&) {
            threwHigh = true;
        }
        assert(threwHigh);

        std::cout << "PASS: Error handling & utility tests\n";
    }

    std::cout << "\nAll PCM 4-bit, 6-bit, and amplitude scaling tests passed successfully!\n";
    return 0;
}
