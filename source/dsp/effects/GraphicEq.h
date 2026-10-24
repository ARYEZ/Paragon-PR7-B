#pragma once

#include "EffectUtils.h"

namespace effects
{
    // A 9-band graphic EQ for bass, each band a peaking filter that can cut
    // or boost by up to 12 dB, plus an output level. The bands are a classic
    // bass amp's (modelled on the Ampeg SVT-3 PRO's graphic EQ): 33 Hz for
    // the sub lows up to 9 kHz for string and slap detail, closer together
    // through the low mids where a bass's tone lives. It sits after the
    // cabinet, where hardware modellers put theirs, to shape the amp and
    // cab's tone before modulation, delay and reverb.
    //
    // It also has that amp's two tone switches, which work whether the
    // faders are switched on or not, as on the amp:
    //   ULTRA LO: a little more deep low end and a deep scoop in the low mids.
    //   ULTRA HI: a big lift in the highs, for slap and pick definition.
    class GraphicEq
    {
    public:
        static constexpr int numBands = 9;
        static constexpr std::array<float, numBands> frequencies { 33.0f, 80.0f, 150.0f, 300.0f, 500.0f,
                                                                   900.0f, 2000.0f, 5000.0f, 9000.0f };
        static constexpr float maxGainDb = 12.0f;

        // The tone switches, as the amp's manual gives them: ULTRA LO is
        // +2 dB at 40 Hz and -10 dB at 500 Hz, ULTRA HI +9 dB at 8 kHz. How
        // wide each one is isn't published: these widths are our choice.
        struct Shape { float frequency, gainDb, q; };
        static constexpr std::array<Shape, 2> ultraLo { { { 40.0f, 2.0f, 0.7f }, { 500.0f, -10.0f, 0.8f } } };
        static constexpr Shape ultraHi { 8000.0f, 9.0f, 0.7f };

        // How wide each band is. 1.2 makes neighbouring bands blend into a
        // smooth curve when several are moved together, as on a hardware
        // graphic EQ, rather than leaving dips between them.
        static constexpr float bandQ = 1.2f;

        struct Settings
        {
            bool on = false;
            std::array<float, numBands> gainsDb {};
            float levelDb = 0.0f;
            bool ultraLo = false, ultraHi = false;
        };

        void prepare (double sampleRate, int maxBlockSize);
        void process (juce::dsp::AudioBlock<float> monoBlock, const Settings&) noexcept;

        // The EQ's response at `frequency` in dB, for drawing its curve.
        static float responseDb (const std::array<float, numBands>& gainsDb, float frequency, double sampleRate = 48000.0,
                                 bool ultraLoOn = false, bool ultraHiOn = false);

        // A band's frequency, kept safely below half the sample rate.
        static float bandFrequency (int band, double sampleRate) noexcept;
        static float safeFrequency (float frequency, double sampleRate) noexcept;

    private:
        double sampleRate = 48000.0;
        std::array<juce::dsp::IIR::Filter<float>, numBands> filters;
        std::array<juce::SmoothedValue<float>, numBands> gains;   // in dB
        std::array<float, numBands> appliedDb {};
        juce::SmoothedValue<float, juce::ValueSmoothingTypes::Multiplicative> level;
        BypassFade bypass;

        std::array<juce::dsp::IIR::Filter<float>, 2> ultraLoFilters;
        juce::dsp::IIR::Filter<float> ultraHiFilter;
        BypassFade ultraLoBypass, ultraHiBypass;
    };
}
