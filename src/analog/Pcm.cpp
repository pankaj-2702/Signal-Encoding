
#include "sigcodec/analog/Pcm.hpp"

#include <cmath>
#include <stdexcept>
#include <string>

namespace sigcodec {

BitStream Pcm::encode(
    const std::vector<double>& samples,
    int bitsPerSample
) const {
    if (bitsPerSample < 1 || bitsPerSample > 16) {
        throw std::invalid_argument(
            "Bits per sample must be between 1 and 16."
        );
    }

    BitStream bits;

    const unsigned int levels = 1U << bitsPerSample;
    const unsigned int maximumCode = levels - 1U;

    bits.reserve(samples.size() *
                 static_cast<std::size_t>(bitsPerSample));

    for (double sample : samples) {
        if (!std::isfinite(sample)) {
            throw std::invalid_argument(
                "Analog samples must be finite numbers."
            );
        }

        // Normalize the input range [-1, +1] to [0, 1].
        double normalized = (sample + 1.0) / 2.0;

        if (normalized < 0.0) {
            normalized = 0.0;
        } else if (normalized > 1.0) {
            normalized = 1.0;
        }

        // Quantize the sample to the nearest PCM level.
        unsigned int code = static_cast<unsigned int>(
            std::round(normalized * maximumCode)
        );

        // Store each quantization code, most significant bit first.
        for (int shift = bitsPerSample - 1; shift >= 0; --shift) {
            bits.push_back(
                static_cast<Bit>((code >> shift) & 1U)
            );
        }
    }

    return bits;
}

PcmResult Pcm::decode(
    const BitStream& bits,
    int bitsPerSample
) const {
    if (bitsPerSample < 1 || bitsPerSample > 16) {
        throw std::invalid_argument(
            "Bits per sample must be between 1 and 16."
        );
    }

    if (bits.size() % static_cast<std::size_t>(bitsPerSample) != 0) {
        throw std::invalid_argument(
            "PCM bit count must be divisible by bits per sample."
        );
    }

    for (Bit bit : bits) {
        if (bit > 1) {
            throw std::invalid_argument(
                "PCM input must contain only zeroes and ones."
            );
        }
    }

    PcmResult result;

    const unsigned int levels = 1U << bitsPerSample;
    const unsigned int maximumCode = levels - 1U;

    const std::size_t sampleCount =
        bits.size() / static_cast<std::size_t>(bitsPerSample);

    result.reconstructedSamples.reserve(sampleCount);
    result.staircase.reserve(sampleCount * 2);

    for (std::size_t i = 0; i < sampleCount; ++i) {
        unsigned int code = 0;

        const std::size_t start =
            i * static_cast<std::size_t>(bitsPerSample);

        for (int j = 0; j < bitsPerSample; ++j) {
            code = (code << 1U) | bits[start + j];
        }

        // Reconstruct the quantized sample in [-1, +1].
        double sample =
            (2.0 * static_cast<double>(code) / maximumCode) - 1.0;

        result.reconstructedSamples.push_back(sample);

        // Store two points per sample to create a staircase waveform.
        result.staircase.push_back({
            static_cast<double>(i),
            sample
        });

        result.staircase.push_back({
            static_cast<double>(i + 1),
            sample
        });
    }

    return result;
}

std::string Pcm::bitsToString(const BitStream& bits) {
    std::string output;
    output.reserve(bits.size());

    for (Bit bit : bits) {
        output.push_back(bit == 0 ? '0' : '1');
    }

    return output;
}

}
