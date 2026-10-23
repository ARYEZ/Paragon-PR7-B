#pragma once

#include "PitchShifter.h"

namespace effects
{
    // The FILTER / PITCH block: after the compressor, before the drive,
    // where a wah or a pitch pedal sits on a real pedalboard. Two types:
    //
    //   autoWah       an envelope filter: a resonant wah filter swept by how
    //                 hard you play (a Mu-Tron III or Boss AW-3 style
    //                 "quack"), rather than by your foot
    //   pitchShifter  shifts the guitar up or down by an interval, from two
    //                 octaves down to two up; blend it with the dry guitar
    //                 for harmonies and octaves, or all wet for a pitch drop
    class FilterPitch
    {
    public:
        // Saved by number in rigs and projects: only ever add to the end.
        enum class Type { autoWah, pitchShifter };
        static inline const juce::StringArray typeNames { "Auto-Wah", "Pitch Shifter" };

        struct Settings
        {
            bool on = false;
            Type type = Type::autoWah;
            float sensitivity = 5.0f;   // 0 to 10: how far a given picking strength opens the wah
            float resonance = 5.0f;     // 0 to 10: how sharp and vocal the wah's peak is
            float semitones = 12.0f;    // -24 to +24
            float mix = 1.0f;           // 0 to 1
        };

        FilterPitch();
        void prepare (double sampleRate, int maxBlockSize);
        void process (juce::dsp::AudioBlock<float> monoBlock, const Settings& settings) noexcept;

        // Exposed for tests: where the wah's filter is for an envelope level (0 to 1).
        static float wahFrequencyFor (float position) noexcept;

    private:
        void processAutoWah (juce::dsp::AudioBlock<float> block, const Settings& settings) noexcept;
        void processPitch (juce::dsp::AudioBlock<float> block, const Settings& settings) noexcept;

        double sampleRate = 48000.0;
        BypassFade bypass;
        juce::SmoothedValue<float> mix;

        // Auto-wah
        juce::dsp::StateVariableTPTFilter<float> wahFilter;
        float envelope = 0.0f, sweep = 0.0f;

        // Pitch shifter
        PitchShifter shifter;
        juce::SmoothedValue<float> ratio;
    };
}
