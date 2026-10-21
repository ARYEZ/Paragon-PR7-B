#include <catch2/catch_test_macros.hpp>

#include <cmath>
#include <filesystem>
#include <vector>

#include "NAM/get_dsp.h"

// Phase 1 only links the NAM engine; Phase 2 wires it into the signal chain.
// This proves the engine builds and runs on every platform before then.
TEST_CASE ("NAM core loads and runs a WaveNet model", "[nam]")
{
    const auto modelPath = std::filesystem::path (PARAGON_NAM_EXAMPLE_MODELS) / "wavenet.nam";
    REQUIRE (std::filesystem::exists (modelPath));

    auto model = nam::get_dsp (modelPath);
    REQUIRE (model != nullptr);

    constexpr int blockSize = 512;
    model->Reset (48000.0, blockSize);

    std::vector<float> input (blockSize), output (blockSize);
    for (int i = 0; i < blockSize; ++i)
        input[(size_t) i] = 0.3f * std::sin (2.0f * 3.14159265f * 220.0f * (float) i / 48000.0f);

    float* inputChannels[] = { input.data() };
    float* outputChannels[] = { output.data() };
    model->process (inputChannels, outputChannels, blockSize);

    float energy = 0.0f;
    for (auto sample : output)
    {
        REQUIRE (std::isfinite (sample));
        energy += sample * sample;
    }
    CHECK (energy > 0.0f);
}
