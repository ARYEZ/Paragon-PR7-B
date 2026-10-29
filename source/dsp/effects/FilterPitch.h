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
    //   whammyWah     the DigiTech XP-100 Whammy-Wah's programs, played with
    //                 the expression pedal (its WHAMMY-WAH setting). PROGRAM
    //                 picks one, numbered as on the XP-100: a volume pedal,
    //                 five wahs, six auto-wahs (the pedal sets their
    //                 sensitivity or attack), eight whammy bends (no shift at
    //                 the heel, the interval at the toe, wet only) and eight
    //                 harmonies (the guitar plus a voice the pedal moves
    //                 between two intervals)
    //   magnetar         a bass envelope filter modelled on the MXR M82, from
    //                 its manual's specifications (no schematic): a resonant
    //                 band-pass swept from DECAY's stop frequency (76 Hz to
    //                 1.3 kHz) up to 3.2 kHz by how hard you play, SENS over
    //                 a 46 dB range, Q from +8 dB to +18/+26 dB of peak, and
    //                 separate DRY and FX levels so the lows stay solid

    // One of the XP-100's programs (see FilterPitch::xpPrograms).
    struct XpProgram
    {
        enum class Kind { volume, wah, autoWah, whammy, harmony };
        const char* name;
        Kind kind;
        float heel = 0.0f, toe = 0.0f;   // wahs: the filter's range in Hz; whammy and harmony: the shift in semitones
        float q = 0.0f;                  // wahs: how sharp the peak is
        bool lowPass = false;            // auto-wahs: a low-pass sweep (envelope filter, brass) rather than a band-pass (wah)
        float attackSeconds = 0.0f, releaseSeconds = 0.0f;   // auto-wahs: how fast the envelope opens and closes
        bool pedalSetsAttack = false;    // auto-wahs: the pedal sets the attack (heel fast, toe slow) rather than the sensitivity

        static constexpr XpProgram volume (const char* name) { return { name, Kind::volume }; }
        static constexpr XpProgram wah (const char* name, float lowHz, float highHz, float q)
        {
            return { name, Kind::wah, lowHz, highHz, q };
        }
        static constexpr XpProgram autoWah (const char* name, float lowHz, float highHz, float q, bool lowPass,
                                            float attack, float release, bool pedalSetsAttack)
        {
            return { name, Kind::autoWah, lowHz, highHz, q, lowPass, attack, release, pedalSetsAttack };
        }
        static constexpr XpProgram whammy (const char* name, float semitones) { return { name, Kind::whammy, 0.0f, semitones }; }
        static constexpr XpProgram harmony (const char* name, float heel, float toe) { return { name, Kind::harmony, heel, toe }; }
    };

    class FilterPitch
    {
    public:
        // Saved by number in rigs and projects: only ever add to the end.
        // (talkBox is processed after the amp by the TalkBox class; here it passes the guitar through.)
        enum class Type { autoWah, pitchShifter, whammyWah, talkBox, magnetar };
        static inline const juce::StringArray typeNames { "Auto-Wah", "Pitch Shifter", "Quasar", "Talk Box", "Magnetar" };

        // The XP-100's factory programs 01 to 28, in its order (program n is
        // index n - 1), with what its pedal does in each. Saved by number:
        // only ever add to the end. The names follow the XP-100's list,
        // shortened to fit ("Sens": the pedal sets the auto-wah's
        // sensitivity; "Attack"/"Atk": its attack time). The voicings
        // (frequency ranges, Q, envelope speeds) are ours: DigiTech didn't
        // publish theirs.
        static inline const std::array<XpProgram, 28> xpPrograms { {
            XpProgram::volume  ("01 Volume Pedal"),
            XpProgram::wah     ("02 Classic Wah", 450.0f, 2200.0f, 5.0f),     // a Cry Baby's range
            XpProgram::wah     ("03 Classic Wah 2", 350.0f, 2800.0f, 6.5f),           // wider and more vocal
            XpProgram::wah     ("04 Dark & Deep", 200.0f, 1200.0f, 6.0f),
            XpProgram::wah     ("05 Sweet Spot 1", 500.0f, 1300.0f, 7.0f),        // just the middle of the sweep, finely
            XpProgram::wah     ("06 Sweet Spot 2", 750.0f, 1900.0f, 7.0f),        // the same, higher up
            XpProgram::autoWah ("07 Wah Auto Sens", 450.0f, 2200.0f, 5.0f, false, 0.004f, 0.2f, false),
            XpProgram::autoWah ("08 Env Filter Sens", 250.0f, 3000.0f, 4.0f, true, 0.01f, 0.15f, false),
            XpProgram::autoWah ("09 Brass Attack Sens", 300.0f, 4000.0f, 2.5f, true, 0.03f, 0.25f, false),
            XpProgram::autoWah ("10 Wah Auto Attack", 450.0f, 2200.0f, 5.0f, false, 0.004f, 0.2f, true),
            XpProgram::autoWah ("11 Env Filter Attack", 250.0f, 3000.0f, 4.0f, true, 0.01f, 0.15f, true),
            XpProgram::autoWah ("12 Brass Attack Atk", 300.0f, 4000.0f, 2.5f, true, 0.03f, 0.25f, true),
            XpProgram::whammy  ("13 Up 1 Octave", 12.0f),
            XpProgram::whammy  ("14 Up 2 Octaves", 24.0f),
            XpProgram::whammy  ("15 Up 2nd", 2.0f),
            XpProgram::whammy  ("16 Down 2nd", -2.0f),
            XpProgram::whammy  ("17 Down 4th", -5.0f),
            XpProgram::whammy  ("18 Down 1 Octave", -12.0f),
            XpProgram::whammy  ("19 Down 2 Octaves", -24.0f),
            XpProgram::whammy  ("20 Down 6 Octaves", -72.0f),                    // a dive bomb, down to a rumble
            XpProgram::harmony ("21 Down Oct / Up Oct", -12.0f, 12.0f),
            XpProgram::harmony ("22 Down 4th / Down 3rd", -5.0f, -4.0f),
            XpProgram::harmony ("23 Up 5th / Up 6th", 7.0f, 9.0f),
            XpProgram::harmony ("24 Up 4th / Up 5th", 5.0f, 7.0f),
            XpProgram::harmony ("25 Up m3rd / Up M3rd", 3.0f, 4.0f),
            XpProgram::harmony ("26 Up 2nd / Up 3rd", 2.0f, 4.0f),
            XpProgram::harmony ("27 Down 4th / Up 5th", -5.0f, 7.0f),
            XpProgram::harmony ("28 Up 5th / Up Oct", 7.0f, 12.0f),
        } };
        static juce::StringArray xpProgramNames();

        struct Settings
        {
            bool on = false;
            Type type = Type::autoWah;
            float sensitivity = 5.0f;   // 0 to 10: how far a given picking strength opens the wah
            float resonance = 5.0f;     // 0 to 10: how sharp and vocal the wah's peak is
            float semitones = 12.0f;    // -24 to +24
            float mix = 1.0f;           // 0 to 1
            int xpProgram = 1;          // an index into xpPrograms (program 02, the Original Cry Wah)
            float pedal = 0.0f;         // the expression pedal for the Whammy-Wah, 0 (heel) to 1 (toe)
            float magnetarDry = 5.0f, magnetarFx = 5.0f, magnetarDecay = 5.0f, magnetarQ = 5.0f, magnetarSens = 5.0f;   // MAGNETAR's knobs, 0 to 10
        };

        FilterPitch();
        void prepare (double sampleRate, int maxBlockSize);
        void process (juce::dsp::AudioBlock<float> monoBlock, const Settings& settings) noexcept;

        // Exposed for tests: where the wah's filter is for an envelope level (0 to 1).
        static float wahFrequencyFor (float position) noexcept;

        // MAGNETAR, from the M82's specifications. For tests and the faceplate.
        static constexpr float magnetarTopHz = 3200.0f, magnetarLowestStopHz = 76.0f, magnetarHighestStopHz = 1300.0f;
        static float magnetarStopHz (float decay) noexcept;                  // DECAY: where the filter rests
        static float magnetarFrequencyFor (float position, float decay) noexcept;   // 0 (at rest) to 1 (fully open)
        static float magnetarPeakDb (float q, float frequencyHz) noexcept;   // the filter's gain at its peak
        static float magnetarDryGain (float dry) noexcept;                   // 0 to 2 (+6 dB)
        static float magnetarFxGain (float fx) noexcept;                     // 0 to 1, of the filter's gain
        // The input level (dBFS, a steady note's peak) that opens the filter fully.
        static float magnetarFullyOpenDb (float sens) noexcept;
        // The output stage's limit: the pedal's +8 dBV maximum output, as
        // a peak, with full scale 1 V (as for the drives).
        static constexpr float magnetarMaxOutput = 3.55f;
        // Where the filter is now (Hz), for the faceplate.
        float getMagnetarFrequency() const noexcept { return magnetarFrequency.load(); }

    private:
        void processMagnetar (juce::dsp::AudioBlock<float> block, const Settings& settings) noexcept;
        void processAutoWah (juce::dsp::AudioBlock<float> block, const Settings& settings) noexcept;
        void processPitch (juce::dsp::AudioBlock<float> block, const Settings& settings) noexcept;
        void processWhammyWah (juce::dsp::AudioBlock<float> block, const Settings& settings) noexcept;

        double sampleRate = 48000.0;
        BypassFade bypass;
        juce::SmoothedValue<float> mix;

        // Auto-wah
        juce::dsp::StateVariableTPTFilter<float> wahFilter;
        float envelope = 0.0f, sweep = 0.0f;

        // Magnetar
        juce::dsp::StateVariableTPTFilter<float> magnetarFilter;
        float magnetarEnvelope = 0.0f, magnetarSweep = 0.0f;
        juce::SmoothedValue<float> magnetarDry, magnetarFx;
        std::atomic<float> magnetarFrequency { magnetarLowestStopHz };

        // Pitch shifter
        PitchShifter shifter;
        juce::SmoothedValue<float> ratio;

        // Whammy-Wah: its pedal, smoothed so a stepped MIDI pedal sweeps
        // and bends continuously.
        juce::SmoothedValue<float> pedal;
    };
}
