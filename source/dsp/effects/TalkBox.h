#pragma once

#include "EffectUtils.h"

namespace effects
{
    // The FILTER / PITCH block's Talk Box type. A real talk box sends the
    // amp's sound up a tube into the player's mouth, which shapes it into
    // vowels for a microphone. This shapes it the same way the mouth does:
    // three resonant peaks (formants) at the frequencies a voice uses for
    // each vowel. The expression pedal (its TALK BOX setting) glides them
    // through oo -> oh -> ah -> eh -> ee, heel to toe. It sits after the amp
    // and cab, as the real thing takes the amp's (distorted) sound, which is
    // full of harmonics for the formants to shape.
    class TalkBox
    {
    public:
        void prepare (double sampleRate, int maxBlockSize);

        // `position` is the pedal, 0 (heel: oo) to 1 (toe: ee). `resonance`,
        // 0 to 10, is how sharp and vocal the formants are.
        void process (juce::dsp::AudioBlock<float> monoBlock, bool on, float position, float resonance, float mix) noexcept;

        // The three formant frequencies for a pedal position (for tests).
        static std::array<float, 3> formantsFor (float position) noexcept;

    private:
        std::array<juce::dsp::StateVariableTPTFilter<float>, 3> formants;
        juce::SmoothedValue<float> mouth, mix;
        BypassFade bypass;
    };
}
