#pragma once

#include "EffectUtils.h"

namespace effects
{
    // The expression pedal's WAH / FILTER mode: a resonant band-pass filter
    // swept by the pedal, from 350 Hz at the heel to 2.2 kHz at the toe
    // (the range of a classic wah). It sits before the drive and amp, which
    // is where a wah goes on a real pedalboard.
    class Wah
    {
    public:
        void prepare (double sampleRate, int maxBlockSize);

        // `position` is the pedal, 0 (heel) to 1 (toe).
        void process (juce::dsp::AudioBlock<float> monoBlock, bool on, float position) noexcept;

        static float frequencyFor (float position) noexcept;

    private:
        juce::dsp::StateVariableTPTFilter<float> filter;
        juce::SmoothedValue<float> sweep;
        BypassFade bypass;
    };
}
