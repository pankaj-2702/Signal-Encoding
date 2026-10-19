
#include "sigcodec/analog/Pcm.hpp"

#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>

int main() {
    sigcodec::Pcm pcm;

    // Test 1: Encode and decode analog samples.
    std::vector<double> samples = {-1.0, 0.0, 1.0};

    auto bits = pcm.encode(samples, 8);
    auto decoded = pcm.decode(bits, 8);

    assert(bits.size() == 24);
    assert(decoded.reconstructedSamples.size() == 3);
    assert(std::abs(decoded.reconstructedSamples[0] + 1.0) < 0.001);
    assert(std::abs(decoded.reconstructedSamples[1]) < 0.01);
    assert(std::abs(decoded.reconstructedSamples[2] - 1.0) < 0.001);

    // Test 2: Check the staircase waveform.
    assert(decoded.staircase.size() == 6);

    // Test 3: Check bit-string conversion.
    assert(sigcodec::Pcm::bitsToString({1, 0, 1, 1}) == "1011");

    std::cout << "All PCM tests passed!\n";
    return 0;
}
