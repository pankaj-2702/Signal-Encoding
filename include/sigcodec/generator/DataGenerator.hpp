#pragma once

#include <cstddef>
#include <random>

#include "sigcodec/core/Types.hpp"

namespace sigcodec {

struct GenerationResult {
    BitStream bits;
    BitStream longestPalindrome;
    std::size_t patternStart{};
};

class DataGenerator {
public:
    DataGenerator();

    // Method 1: Generate a completely random bitstream.
    GenerationResult generateRandom(std::size_t length);

    // Method 2: Generate random bits and insert a required pattern.
    GenerationResult generateWithPattern(
        std::size_t length,
        const BitStream& pattern
    );

    // Method 3: Process custom bitstream and compute longest palindrome.
    GenerationResult createCustom(const BitStream& bits);
    GenerationResult createCustom(const std::string& bitString);

private:
    std::mt19937 randomEngine_;

    BitStream generateRandomBits(std::size_t length);
    BitStream findLongestPalindrome(const BitStream& bits) const;
};

} // namespace sigcodec