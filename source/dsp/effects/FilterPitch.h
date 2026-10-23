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
    //   whammyWah     a DigiTech XP-100 Whammy-Wah style pedal effect, played
    //                 with the expression pedal (its WHAMMY-WAH setting). Its
    //                 MODE picks one of three kinds, as on the XP-100:
    //                   Wah      a wah swept by the pedal, in five voicings
    //                   Whammy   the pedal bends the pitch, from none at the
    //                            heel to the mode's interval at the toe (wet only)
    //                   Harmony  the guitar plus a second voice, which the
    //                            pedal moves between two intervals
    // One of the FILTER / PITCH block's Whammy-Wah modes (see below).
    struct WhammyWahMode
    {
        enum class Kind { wah, whammy, harmony };
        const char* name;
        Kind kind;
        float heel, toe;      // wah: the filter's frequency in Hz; whammy and harmony: the shift in semitones
        float q = 0.0f;       // wah only: how sharp its peak is
        float curve = 1.0f;   // whammy: how the bend follows the pedal (2 for a dive bomb's late plunge)
    };

    class FilterPitch
    {
    public:
        // Saved by number in rigs and projects: only ever add to the end.
        enum class Type { autoWah, pitchShifter, whammyWah };
        static inline const juce::StringArray typeNames { "Auto-Wah", "Pitch Shifter", "Whammy-Wah" };

        // The Whammy-Wah's modes. Saved by number: only ever add to the end.
        using WhammyWahMode = effects::WhammyWahMode;
        static inline const std::array<WhammyWahMode, 22> whammyWahModes { {
            { "Wah: Classic",          WhammyWahMode::Kind::wah, 400.0f, 2200.0f, 5.0f },
            { "Wah: Vocal",            WhammyWahMode::Kind::wah, 450.0f, 2000.0f, 9.0f },
            { "Wah: Wide",             WhammyWahMode::Kind::wah, 250.0f, 3500.0f, 6.0f },
            { "Wah: Deep",             WhammyWahMode::Kind::wah, 200.0f, 1200.0f, 6.0f },
            { "Wah: Bright",           WhammyWahMode::Kind::wah, 600.0f, 3800.0f, 4.0f },
            { "Whammy: 2 Oct Up",      WhammyWahMode::Kind::whammy, 0.0f, 24.0f },
            { "Whammy: Oct Up",        WhammyWahMode::Kind::whammy, 0.0f, 12.0f },
            { "Whammy: 5th Down",      WhammyWahMode::Kind::whammy, 0.0f, -7.0f },
            { "Whammy: 4th Down",      WhammyWahMode::Kind::whammy, 0.0f, -5.0f },
            { "Whammy: Oct Down",      WhammyWahMode::Kind::whammy, 0.0f, -12.0f },
            { "Whammy: 2 Oct Down",    WhammyWahMode::Kind::whammy, 0.0f, -24.0f },
            { "Whammy: Dive Bomb",     WhammyWahMode::Kind::whammy, 0.0f, -24.0f, 0.0f, 2.0f },
            { "Whammy: Drop Tune",     WhammyWahMode::Kind::whammy, 0.0f, -2.0f },
            { "Harmony: 2nd / 3rd Up", WhammyWahMode::Kind::harmony, 2.0f, 4.0f },
            { "Harmony: b3rd / 3rd Up", WhammyWahMode::Kind::harmony, 3.0f, 4.0f },
            { "Harmony: 3rd / 4th Up", WhammyWahMode::Kind::harmony, 4.0f, 5.0f },
            { "Harmony: 4th / 5th Up", WhammyWahMode::Kind::harmony, 5.0f, 7.0f },
            { "Harmony: 5th / 6th Up", WhammyWahMode::Kind::harmony, 7.0f, 9.0f },
            { "Harmony: 5th / Oct Up", WhammyWahMode::Kind::harmony, 7.0f, 12.0f },
            { "Harmony: 3rd / 4th Down", WhammyWahMode::Kind::harmony, -4.0f, -5.0f },
            { "Harmony: 4th / 5th Down", WhammyWahMode::Kind::harmony, -5.0f, -7.0f },
            { "Harmony: Oct Down / Up", WhammyWahMode::Kind::harmony, -12.0f, 12.0f },
        } };
        static juce::StringArray whammyWahModeNames();

        struct Settings
        {
            bool on = false;
            Type type = Type::autoWah;
            float sensitivity = 5.0f;   // 0 to 10: how far a given picking strength opens the wah
            float resonance = 5.0f;     // 0 to 10: how sharp and vocal the wah's peak is
            float semitones = 12.0f;    // -24 to +24
            float mix = 1.0f;           // 0 to 1
            int whammyWahMode = 0;      // an index into whammyWahModes
            float pedal = 0.0f;         // the expression pedal for the Whammy-Wah, 0 (heel) to 1 (toe)
        };

        FilterPitch();
        void prepare (double sampleRate, int maxBlockSize);
        void process (juce::dsp::AudioBlock<float> monoBlock, const Settings& settings) noexcept;

        // Exposed for tests: where the wah's filter is for an envelope level (0 to 1).
        static float wahFrequencyFor (float position) noexcept;

    private:
        void processAutoWah (juce::dsp::AudioBlock<float> block, const Settings& settings) noexcept;
        void processPitch (juce::dsp::AudioBlock<float> block, const Settings& settings) noexcept;
        void processWhammyWah (juce::dsp::AudioBlock<float> block, const Settings& settings) noexcept;

        double sampleRate = 48000.0;
        BypassFade bypass;
        juce::SmoothedValue<float> mix;

        // Auto-wah
        juce::dsp::StateVariableTPTFilter<float> wahFilter;
        float envelope = 0.0f, sweep = 0.0f;

        // Pitch shifter
        PitchShifter shifter;
        juce::SmoothedValue<float> ratio;

        // Whammy-Wah: its pedal, smoothed so a stepped MIDI pedal sweeps
        // and bends continuously.
        juce::SmoothedValue<float> pedal;
    };
}
