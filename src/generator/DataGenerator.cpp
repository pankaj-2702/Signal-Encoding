
#include "sigcodec/generator/DataGenerator.hpp"

#include <algorithm>
#include <chrono>
#include <stdexcept>

namespace sigcodec {

DataGenerator::DataGenerator()
    : randomEngine_(
          static_cast<unsigned int>(
              std::chrono::steady_clock::now()
                  .time_since_epoch()
                  .count()
          )
      ) {}

BitStream DataGenerator::generateRandomBits(std::size_t length) {
    BitStream bits;
    bits.reserve(length);

    std::uniform_int_distribution<int> distribution(0, 1);

    for (std::size_t i = 0; i < length; ++i) {
        bits.push_back(
            static_cast<Bit>(distribution(randomEngine_))
        );
    }

    return bits;
}

GenerationResult DataGenerator::generateRandom(std::size_t length) {
    GenerationResult result;

    result.bits = generateRandomBits(length);
    result.longestPalindrome = findLongestPalindrome(result.bits);
    result.patternStart = 0;

    return result;
}

GenerationResult DataGenerator::generateWithPattern(
    std::size_t length,
    const BitStream& pattern
) {
    if (pattern.empty()) {
        throw std::invalid_argument("Pattern cannot be empty.");
    }

    if (pattern.size() > length) {
        throw std::invalid_argument(
            "Pattern length cannot exceed total sequence length."
        );
    }

    for (Bit bit : pattern) {
        if (bit > 1) {
            throw std::invalid_argument(
                "Pattern must contain only 0 and 1."
            );
        }
    }

    GenerationResult result;
    result.bits = generateRandomBits(length);

    // Choose a valid random starting position.
    std::uniform_int_distribution<std::size_t> positionDistribution(
        0, length - pattern.size()
    );

    result.patternStart = positionDistribution(randomEngine_);

    // Insert the required pattern into the random sequence.
    std::copy(
        pattern.begin(),
        pattern.end(),
        result.bits.begin() + result.patternStart
    );

    result.longestPalindrome = findLongestPalindrome(result.bits);

    return result;
}

GenerationResult DataGenerator::createCustom(const BitStream& bits) {
    if (bits.empty()) {
        throw std::invalid_argument("Custom bit sequence cannot be empty.");
    }

    for (Bit bit : bits) {
        if (bit > 1) {
            throw std::invalid_argument("Custom bits must contain only 0 and 1.");
        }
    }

    GenerationResult result;
    result.bits = bits;
    result.longestPalindrome = findLongestPalindrome(result.bits);
    result.patternStart = 0;

    return result;
}

GenerationResult DataGenerator::createCustom(const std::string& bitString) {
    if (bitString.empty()) {
        throw std::invalid_argument("Custom bit sequence cannot be empty.");
    }

    BitStream bits;
    bits.reserve(bitString.size());

    for (char c : bitString) {
        if (c == '0') {
            bits.push_back(0);
        } else if (c == '1') {
            bits.push_back(1);
        } else if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            continue; // allow whitespace separation for readability
        } else {
            throw std::invalid_argument(
                std::string("Invalid character '") + c + "'. Only 0 and 1 are allowed."
            );
        }
    }

    return createCustom(bits);
}

BitStream DataGenerator::findLongestPalindrome(
    const BitStream& bits
) const {
    if (bits.empty()) {
        return {};
    }

    std::size_t bestStart = 0;
    std::size_t bestLength = 1;

    auto expandAroundCenter = [&](int left, int right) {
        while (left >= 0 &&
               right < static_cast<int>(bits.size()) &&
               bits[left] == bits[right]) {
            const std::size_t currentLength =
                static_cast<std::size_t>(right - left + 1);

            if (currentLength > bestLength) {
                bestStart = static_cast<std::size_t>(left);
                bestLength = currentLength;
            }

            --left;
            ++right;
        }
    };

    for (std::size_t i = 0; i < bits.size(); ++i) {
        // Odd-length palindromes.
        expandAroundCenter(
            static_cast<int>(i),
            static_cast<int>(i)
        );

        // Even-length palindromes.
        expandAroundCenter(
            static_cast<int>(i),
            static_cast<int>(i) + 1
        );
    }

    return BitStream(
        bits.begin() + bestStart,
        bits.begin() + bestStart + bestLength
    );
}

} // namespace sigcodec
