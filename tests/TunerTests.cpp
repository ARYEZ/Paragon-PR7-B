#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "dsp/InputTap.h"
#include "dsp/PitchDetector.h"
#include "dsp/effects/Wah.h"

using Catch::Matchers::WithinAbs;

namespace
{
    double centsBetween (double measured, double expected) { return 1200.0 * std::log2 (measured / expected); }

    // A plucked-string-like tone: a fundamental plus decaying overtones,
    // with the 2nd harmonic louder than the fundamental (as a guitar's often
    // is), which is exactly what fools simple tuners into reading an octave up.
    std::vector<float> guitarTone (double frequency, double sampleRate, int numSamples)
    {
        std::vector<float> samples ((size_t) numSamples);
        const double gains[] = { 0.5, 0.8, 0.4, 0.3, 0.2, 0.1 };
        for (int i = 0; i < numSamples; ++i)
        {
            double value = 0.0;
            for (int h = 0; h < 6; ++h)
                value += gains[h] * std::sin (juce::MathConstants<double>::twoPi * frequency * (h + 1) * i / sampleRate + h);
            samples[(size_t) i] = (float) (0.3 * value);
        }
        return samples;
    }
}

TEST_CASE ("Tuner reads guitar notes within a cent at common sample rates", "[tuner]")
{
    const auto sampleRate = GENERATE (44100.0, 48000.0, 96000.0);
    // Low B (7-string), the six open strings, and a high fret on the top string.
    const auto frequency = GENERATE (30.87, 61.74, 82.41, 110.0, 146.83, 196.0, 246.94, 329.63, 987.77);

    PitchDetector detector;
    detector.prepare (sampleRate);
    const auto tone = guitarTone (frequency, sampleRate, detector.windowSizeAtHostRate() + 1000);

    const auto result = detector.detect (tone.data(), (int) tone.size());
    INFO ("sample rate " << sampleRate << ", note " << frequency << " Hz");
    REQUIRE (result.has_value());
    CHECK (std::abs (centsBetween (result->frequency, frequency)) < 1.0);
    CHECK (result->clarity > 0.8);
}

TEST_CASE ("Tuner shows nothing for silence or noise", "[tuner]")
{
    PitchDetector detector;
    detector.prepare (48000.0);
    std::vector<float> samples ((size_t) detector.windowSizeAtHostRate(), 0.0f);
    CHECK_FALSE (detector.detect (samples.data(), (int) samples.size()).has_value());

    juce::Random random (42);
    for (auto& s : samples)
        s = random.nextFloat() * 0.5f - 0.25f;
    CHECK_FALSE (detector.detect (samples.data(), (int) samples.size()).has_value());

    // Not enough audio yet: no reading rather than a wrong one.
    CHECK_FALSE (detector.detect (samples.data(), 100).has_value());
}

TEST_CASE ("Tuner names notes and cents", "[tuner]")
{
    auto note = PitchDetector::toNote (440.0);
    CHECK (note.name == "A");
    CHECK (note.octave == 4);
    CHECK_THAT (note.cents, WithinAbs (0.0, 1.0e-9));

    note = PitchDetector::toNote (82.41);
    CHECK (note.name == "E");
    CHECK (note.octave == 2);
    CHECK (std::abs (note.cents) < 1.0);

    note = PitchDetector::toNote (440.0 * std::pow (2.0, 0.2 / 12.0));   // A4 + 20 cents
    CHECK (note.name == "A");
    CHECK_THAT (note.cents, WithinAbs (20.0, 1.0e-6));

    note = PitchDetector::toNote (466.16 * std::pow (2.0, -0.3 / 12.0)); // A#4 - 30 cents
    CHECK (note.name == "A#");
    CHECK_THAT (note.cents, WithinAbs (-30.0, 0.1));

    note = PitchDetector::toNote (30.87);
    CHECK (note.name == "B");
    CHECK (note.octave == 0);

    // A different reference pitch moves everything with it.
    note = PitchDetector::toNote (432.0, 432.0);
    CHECK (note.name == "A");
    CHECK_THAT (note.cents, WithinAbs (0.0, 1.0e-9));
}

TEST_CASE ("Input tap hands the newest audio to another thread in order", "[tuner]")
{
    InputTap tap (8);
    const float first[] = { 1, 2, 3, 4, 5 };
    const float second[] = { 6, 7, 8, 9, 10, 11 };   // wraps around the ring
    tap.write (first, 5);
    tap.write (second, 6);

    float latest[4] {};
    tap.readLatest (latest, 4);
    CHECK (latest[0] == 8.0f);
    CHECK (latest[3] == 11.0f);
}

TEST_CASE ("Wah sweeps a band-pass from heel to toe", "[wah]")
{
    CHECK_THAT (effects::Wah::frequencyFor (0.0f), WithinAbs (350.0, 0.01));
    CHECK_THAT (effects::Wah::frequencyFor (1.0f), WithinAbs (2200.0, 0.01));
    CHECK (effects::Wah::frequencyFor (0.5f) > 800.0f);
    CHECK (effects::Wah::frequencyFor (0.5f) < 900.0f);

    // Measure how loud a tone comes through with the pedal at each end.
    auto levelThrough = [] (double toneHz, float position)
    {
        constexpr double sampleRate = 48000.0;
        constexpr int blockSize = 256;
        effects::Wah wah;
        wah.prepare (sampleRate, blockSize);
        float peak = 0.0f;
        std::vector<float> block (blockSize);
        for (int b = 0; b < 200; ++b)
        {
            for (int i = 0; i < blockSize; ++i)
                block[(size_t) i] = (float) std::sin (juce::MathConstants<double>::twoPi * toneHz * (b * blockSize + i) / sampleRate);
            float* channels[] = { block.data() };
            wah.process (juce::dsp::AudioBlock<float> (channels, 1, blockSize), true, position);
            if (b > 100)
                for (auto s : block)
                    peak = std::max (peak, std::abs (s));
        }
        return peak;
    };

    // Heel down favours the low tone, toe down the high tone.
    CHECK (levelThrough (350.0, 0.0f) > 2.0f * levelThrough (2200.0, 0.0f));
    CHECK (levelThrough (2200.0, 1.0f) > 2.0f * levelThrough (350.0, 1.0f));
    // At its centre frequency the make-up gain keeps the level roughly where it was.
    CHECK (levelThrough (350.0, 0.0f) > 0.7f);
    CHECK (levelThrough (350.0, 0.0f) < 2.0f);
}
