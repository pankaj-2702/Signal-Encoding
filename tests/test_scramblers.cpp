
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>

#include "sigcodec/linecode/B8ZS.hpp"
#include "sigcodec/linecode/HDB3.hpp"

using namespace sigcodec;

int passed = 0;

void check(
    bool condition,
    const std::string& description
) {
    if (!condition) {
        std::cerr << "[FAIL] " << description << '\n';
        std::exit(1);
    }

    ++passed;
    std::cout << "[PASS] " << description << '\n';
}

void checkSignal(
    const BipolarSignal& actual,
    const BipolarSignal& expected,
    const std::string& description
) {
    check(actual == expected, description);
}

void testB8ZS() {
    B8ZS coder;

    // Positive preceding pulse.
    checkSignal(
        coder.encode({1, 0, 0, 0, 0, 0, 0, 0, 0, 1}),
        {+1, 0, 0, 0, +1, -1, 0, -1, +1, -1},
        "B8ZS substitution after positive pulse"
    );

    // Negative preceding pulse.
    checkSignal(
        coder.encode({1, 1, 0, 0, 0, 0, 0, 0, 0, 0}),
        {+1, -1, 0, 0, 0, -1, +1, 0, +1, -1},
        "B8ZS substitution after negative pulse"
    );

    // No run of eight zeros.
    checkSignal(
        coder.encode({1, 0, 0, 1, 0}),
        {+1, 0, 0, -1, 0},
        "B8ZS leaves short zero runs unchanged"
    );

    // All zeros, with no preceding pulse.
    checkSignal(
    coder.encode({0, 0, 0, 0, 0, 0, 0, 0}),
    {0, 0, 0, -1, +1, 0, +1, -1},
    "B8ZS handles eight zeros at the beginning"
);
}

void testHDB3() {
    HDB3 coder;

    // Odd number of ordinary pulses: 000V.
    checkSignal(
        coder.encode({1, 0, 0, 0, 0}),
        {+1, 0, 0, 0, +1},
        "HDB3 uses 000V for odd pulse count"
    );

    // Even number of ordinary pulses: B00V.
    checkSignal(
        coder.encode({1, 1, 0, 0, 0, 0}),
        {+1, -1, +1, 0, 0, +1},
        "HDB3 uses B00V for even pulse count"
    );

    // Two consecutive substitutions.
    checkSignal(
        coder.encode({1, 0, 0, 0, 0, 0, 0, 0, 0, 1}),
        {+1, 0, 0, 0, +1, -1, 0, 0, -1, +1},
        "HDB3 handles consecutive four-zero groups"
    );

    
checkSignal(
    coder.encode({0, 0, 0, 0, 1}),
    {0, 0, 0, -1, +1},
    "HDB3 handles four zeros at the beginning"
);
}

template <typename Coder>
void testInvalidBits(Coder& coder, const std::string& name) {
    bool rejected = false;

    try {
        coder.encode({1, 0, 2, 1});
    } catch (const std::invalid_argument&) {
        rejected = true;
    }

    check(rejected, name + " rejects invalid input bits");
}

int main() {
    std::cout << "=== B8ZS Tests ===\n";
    testB8ZS();

    std::cout << "\n=== HDB3 Tests ===\n";
    testHDB3();

    B8ZS b8zs;
    HDB3 hdb3;

    std::cout << "\n=== Invalid Input Tests ===\n";
    testInvalidBits(b8zs, "B8ZS");
    testInvalidBits(hdb3, "HDB3");

    std::cout << "\nAll " << passed << " tests passed!\n";
    return 0;
}
