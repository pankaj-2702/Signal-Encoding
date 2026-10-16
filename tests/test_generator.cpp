
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>

#include "sigcodec/generator/DataGenerator.hpp"

using namespace sigcodec;

int testsPassed = 0;

void check(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "[FAIL] " << message << '\n';
        std::exit(1);
    }

    ++testsPassed;
    std::cout << "[PASS] " << message << '\n';
}

bool isPalindrome(const BitStream& bits) {
    return std::equal(
        bits.begin(),
        bits.begin() + bits.size() / 2,
        bits.rbegin()
    );
}

bool isSubstring(
    const BitStream& sequence,
    const BitStream& candidate
) {
    return std::search(
        sequence.begin(), sequence.end(),
        candidate.begin(), candidate.end()
    ) != sequence.end();
}

std::size_t bruteForceLongestPalindrome(const BitStream& bits) {
    std::size_t best = 0;

    for (std::size_t start = 0; start < bits.size(); ++start) {
        for (std::size_t length = 1;
             start + length <= bits.size();
             ++length) {

            BitStream candidate(
                bits.begin() + start,
                bits.begin() + start + length
            );

            if (isPalindrome(candidate)) {
                best = std::max(best, length);
            }
        }
    }

    return best;
}

void testRandomGeneration(DataGenerator& generator) {
    auto empty = generator.generateRandom(0);

    check(empty.bits.empty(), "Random generation: zero length");
    check(empty.longestPalindrome.empty(),
          "Empty input has an empty palindrome");

    for (std::size_t length : {1u, 2u, 5u, 20u, 100u}) {
        auto result = generator.generateRandom(length);

        check(result.bits.size() == length,
              "Random sequence has requested length " +
              std::to_string(length));

        bool validBits = std::all_of(
            result.bits.begin(),
            result.bits.end(),
            [](Bit bit) { return bit == 0 || bit == 1; }
        );

        check(validBits, "Random sequence contains only binary bits");

        check(
            result.longestPalindrome.empty() ||
            isPalindrome(result.longestPalindrome),
            "Reported palindrome reads the same both ways"
        );

        check(
            isSubstring(result.bits, result.longestPalindrome),
            "Reported palindrome exists in generated sequence"
        );

        check(
            result.longestPalindrome.size() ==
                bruteForceLongestPalindrome(result.bits),
            "Reported palindrome is maximal for length " +
            std::to_string(length)
        );
    }
}

void testFixedPatterns(DataGenerator& generator) {
    for (std::size_t zeroCount : {4u, 8u}) {
        const std::size_t length = 30;

        BitStream pattern(zeroCount, 0);

        auto result = generator.generateWithPattern(length, pattern);

        check(
            result.bits.size() == length,
            std::to_string(zeroCount) +
            "-zero pattern: requested total length is preserved"
        );

        check(
            result.patternStart + zeroCount <= result.bits.size(),
            "Inserted pattern position is within bounds"
        );

        bool allZeros = true;

        for (std::size_t i = 0; i < zeroCount; ++i) {
            if (result.bits[result.patternStart + i] != 0) {
                allZeros = false;
                break;
            }
        }

        check(
            allZeros,
            std::to_string(zeroCount) +
            " consecutive zeros are inserted"
        );

        check(
            result.longestPalindrome.size() ==
                bruteForceLongestPalindrome(result.bits),
            "Hybrid generation returns a longest palindrome"
        );
    }
}

void testInvalidPatterns(DataGenerator& generator) {
    bool emptyPatternRejected = false;

    try {
        generator.generateWithPattern(10, {});
    } catch (const std::invalid_argument&) {
        emptyPatternRejected = true;
    }

    check(emptyPatternRejected, "Empty pattern is rejected");

    bool oversizedPatternRejected = false;

    try {
        generator.generateWithPattern(
            3, BitStream{0, 0, 0, 0}
        );
    } catch (const std::invalid_argument&) {
        oversizedPatternRejected = true;
    }

    check(
        oversizedPatternRejected,
        "Pattern longer than sequence is rejected"
    );

    bool invalidBitRejected = false;

    try {
        generator.generateWithPattern(
            5, BitStream{0, 2, 0}
        );
    } catch (const std::invalid_argument&) {
        invalidBitRejected = true;
    }

    check(invalidBitRejected, "Pattern containing 2 is rejected");

    bool zeroLengthRejected = false;

    try {
        generator.generateWithPattern(0, BitStream{0});
    } catch (const std::invalid_argument&) {
        zeroLengthRejected = true;
    }

    check(zeroLengthRejected, "Pattern cannot fit in zero-length sequence");
}

int main() {
    DataGenerator generator;

    std::cout << "\n=== Random Generation Tests ===\n";
    testRandomGeneration(generator);

    std::cout << "\n=== Fixed Pattern Tests ===\n";
    testFixedPatterns(generator);

    std::cout << "\n=== Invalid Input Tests ===\n";
    testInvalidPatterns(generator);

    std::cout << "\nAll " << testsPassed
              << " checks passed!\n";

    return 0;
}
